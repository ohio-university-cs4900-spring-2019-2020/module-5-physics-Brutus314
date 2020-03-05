#include "GLViewPhysicsModule.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "AftrGLRendererBase.h"

//If we want to use way points, we need to include this.
#include "PhysicsModuleWayPoints.h"

#include "WORigidActor.h"

// Include the physics manager
#include "ManagerPhysics.h"

// Net Message includes
#include "NetMsg.h"
#include "NetMessengerClient.h"
#include "NetMsgNewSharedObject.h"
#include "NetMsgMoveSphere.h"
#include "NetMsgObjectOrientation.h"

using namespace Aftr;
using namespace physx;

GLViewPhysicsModule* GLViewPhysicsModule::New( const std::vector< std::string >& args )
{
   GLViewPhysicsModule* glv = new GLViewPhysicsModule( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}


GLViewPhysicsModule::GLViewPhysicsModule( const std::vector< std::string >& args ) : GLView( args )
{
   //Initialize any member variables that need to be used inside of LoadMap() here.
   //Note: At this point, the Managers are not yet initialized. The Engine initialization
   //occurs immediately after this method returns (see GLViewPhysicsModule::New() for
   //reference). Then the engine invoke's GLView::loadMap() for this module.
   //After loadMap() returns, GLView::onCreate is finally invoked.

   //The order of execution of a module startup:
   //GLView::New() is invoked:
   //    calls GLView::init()
   //       calls GLView::loadMap() (as well as initializing the engine's Managers)
   //    calls GLView::onCreate()

   //GLViewPhysicsModule::onCreate() is invoked after this module's LoadMap() is completed.
}

// Overload the init method to initialize new managers
void GLViewPhysicsModule::init(float gravityScalar,
	Vector gravityNormalizedVector,
	std::string confFileName,
	const PHYSICS_ENGINE_TYPE& physicsEngineType)
{
	ManagerPhysics::init();
	GLView::init(gravityScalar, gravityNormalizedVector, confFileName, physicsEngineType);
}

// Overload the shutdown method to shutdown new managers
void GLViewPhysicsModule::shutdownEngine() {
	ManagerPhysics::shutdown();
	GLView::shutdownEngine();
}


void GLViewPhysicsModule::onCreate()
{
   //GLViewPhysicsModule::onCreate() is invoked after this module's LoadMap() is completed.
   //At this point, all the managers are initialized. That is, the engine is fully initialized.

   if( this->pe != NULL )
   {
      //optionally, change gravity direction and magnitude here
      //The user could load these values from the module's aftr.conf
      this->pe->setGravityNormalizedVector( Vector( 0,0,-1.0f ) );
      this->pe->setGravityScalar( Aftr::GRAVITY );
   }
   this->setActorChaseType( STANDARDEZNAV ); //Default is STANDARDEZNAV mode
   //this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1

	// Set up the NetMessenger Client. Must be done here due to reliance on the managers
   if (ManagerEnvironmentConfiguration::getVariableValue("NetServerListenPort") == "12683") {
	   client = NetMessengerClient::New("127.0.0.1", "12682");
   }
   else {
	   client = NetMessengerClient::New("127.0.0.1", "12683");
   }
}


GLViewPhysicsModule::~GLViewPhysicsModule()
{
   //Implicitly calls GLView::~GLView()
}


void GLViewPhysicsModule::updateWorld()
{
   GLView::updateWorld(); //Just call the parent's update world first.
                          //If you want to add additional functionality, do it after
                          //this call.
   // Step the PhysX world
   // Since the getTime returns a value in milliseconds and PhysX expects seconds, divide by 1000
   ManagerPhysics::scene->simulate(ManagerSDLTime::getTimeSinceLastPhysicsIteration() / 1000.0f);
   ManagerPhysics::scene->fetchResults(true);

   PxU32 num_transforms = 0;
   PxActor** activeActors = ManagerPhysics::scene->getActiveActors(num_transforms);
   for (PxU32 i = 0; i < num_transforms; i++) {
	   WORigidActor* bound_data = static_cast<WORigidActor*>(activeActors[i]->userData); // Get the pair
	   PxTransform t = bound_data->actor->getGlobalPose(); // Get the transform
	   PxMat44 new_pose = PxMat44(t); // Get the physx pose matrix
	   Mat4 old_pose = bound_data->wo->getDisplayMatrix();
	   // Collect the new pose into an aftr-acceptable state
	   float convert[16] = {new_pose(0,0),new_pose(0,1), new_pose(0,2),old_pose[3],
							new_pose(1,0),new_pose(1,1), new_pose(1,2),old_pose[7],
							new_pose(2,0),new_pose(2,1), new_pose(2,2),old_pose[11],
							old_pose[12], old_pose[13], old_pose[14], old_pose[15]};

	   Mat4 aftr_mat(convert);
	   // Send orientation net message to update an item's orientation
	   NetMsgObjectOrientation msg;
	   msg.location = Vector(new_pose(0, 3), new_pose(1, 3), new_pose(2, 3));
	   msg.orientation = aftr_mat;
	   msg.wo_index = placed_cubes[bound_data->wo]; // Map data won't change like the worldLst
	   client->sendNetMsgSynchronousTCP(msg);

	   // Apply the new pose
	   bound_data->wo->getModel()->setDisplayMatrix(aftr_mat);
	   bound_data->wo->setPosition(new_pose(0, 3), new_pose(1, 3), new_pose(2, 3));
   }
}


void GLViewPhysicsModule::onResizeWindow( GLsizei width, GLsizei height )
{
   GLView::onResizeWindow( width, height ); //call parent's resize method.
}


void GLViewPhysicsModule::onMouseDown( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseDown( e );
}


void GLViewPhysicsModule::onMouseUp( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseUp( e );
}


void GLViewPhysicsModule::onMouseMove( const SDL_MouseMotionEvent& e )
{
   GLView::onMouseMove( e );
}


void GLViewPhysicsModule::onKeyDown( const SDL_KeyboardEvent& key )
{
   static PxMaterial* gMaterial = ManagerPhysics::gPhysics->createMaterial(0.5f, 0.3f, 0.2f);
   static Vector dropPos = Vector(20, 20, 100);
   static int cubes = 0;
   GLView::onKeyDown( key );
   if( key.keysym.sym == SDLK_0 )
      this->setNumPhysicsStepsPerRender( 1 );

   // Set the drop zone
   if( key.keysym.sym == SDLK_1 )
   {
	   dropPos = this->cam->getPosition();
	   track_sphere->setPosition(dropPos);

	   // Send move sphere net message to update an item's location
	   NetMsgMoveSphere msg;
	   msg.location = dropPos;
	   client->sendNetMsgSynchronousTCP(msg);
   }
   // Make a new object at drop zone
   if (key.keysym.sym == SDLK_2){
   
	   // Add the item to the aftr world
	   WO* wo = WO::New(ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl", Vector(1, 1, 1));
	   wo->setPosition(dropPos);
	   worldLst->push_back(wo);

	   // Send new object net message to spawn a new object
	   NetMsgNewSharedObject msg;
	   msg.location = dropPos;
	   msg.size_scale = Vector(1, 1, 1);
	   msg.model_path = ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl";
	   client->sendNetMsgSynchronousTCP(msg);

	   // Add the item to the physx world
	   PxTransform t = PxTransform(PxVec3(dropPos.x, dropPos.y, dropPos.z));
	   PxShape* shape = ManagerPhysics::gPhysics->createShape(PxBoxGeometry(2.0f, 2.0f, 2.0f), *gMaterial);
	   PxRigidDynamic* actor = PxCreateDynamic(*ManagerPhysics::gPhysics, t, *shape, 10.0f);

	   WORigidActor* combo = WORigidActor::New(wo, actor);
	   ManagerPhysics::addActorBind(combo, actor); // Physx knows its aftr counterpart

	   // Register it to the local storage
	   placed_cubes.insert(std::pair(wo, cubes));
	   cubes++;
   }
}


void GLViewPhysicsModule::onKeyUp( const SDL_KeyboardEvent& key )
{
   GLView::onKeyUp( key );
}


void Aftr::GLViewPhysicsModule::loadMap()
{
   this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();

   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = true;
   this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+

   this->cam->setPosition( 15,15,10 );

   std::string shinyRedPlasticCube( ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl" );
   std::string wheeledCar( ManagerEnvironmentConfiguration::getSMM() + "/models/rcx_treads.wrl" );
   std::string grass( ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl" );
   std::string human( ManagerEnvironmentConfiguration::getSMM() + "/models/human_chest.wrl" );
   std::string yellowSphere(ManagerEnvironmentConfiguration::getLMM() + "/models/sphereYellow.wrl");
   
   //SkyBox Textures readily available
   std::vector< std::string > skyBoxImageNames; //vector to store texture paths
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_winter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/early_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_afternoon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy3+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day2+6.jpg" );
   skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_deepsun+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_evening+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning2+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_noon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_warp+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_Hubble_Nebula+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_gray_matter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_easter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_hot_nebula+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_ice_field+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_lemon_lime+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_milk_chocolate+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_solar_bloom+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg" );

   float ga = 0.1f; //Global Ambient Light level for this module
   ManagerLight::setGlobalAmbientLight( aftrColor4f( ga, ga, ga, 1.0f ) );
   WOLight* light = WOLight::New();
   light->isDirectionalLight( true );
   light->setPosition( Vector( 0, 0, 100 ) );
   //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
   //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
   light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat( { 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD ) );
   light->setLabel( "Light" );
   worldLst->push_back( light );

   //Create the SkyBox
   WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
   wo->setPosition( Vector( 0,0,0 ) );
   wo->setLabel( "Sky Box" );
   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   worldLst->push_back( wo );

   ////Create the infinite grass plane (the floor)
   wo = WO::New( grass, Vector( 5, 5, 1 ), MESH_SHADING_TYPE::mstFLAT );
   wo->setPosition( Vector( 0, 0, 0 ) );
   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 );
   grassSkin.getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
   grassSkin.setAmbient( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Color of object when it is not in any light
   grassSkin.setDiffuse( aftrColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ); //Diffuse color components (ie, matte shading color of this object)
   grassSkin.setSpecular( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Specular color component (ie, how "shiney" it is)
   grassSkin.setSpecularCoefficient( 10 ); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
   wo->setLabel( "Grass" );
   worldLst->push_back( wo );
   // Add this plane to the PhysX engine
   PxMaterial* gMaterial = ManagerPhysics::gPhysics->createMaterial(0.5f, 0.3f, 0.2f);
   PxRigidStatic* groundPlane = PxCreatePlane(*ManagerPhysics::gPhysics, PxPlane(0, 0, 1, 0), *gMaterial);
   ManagerPhysics::addActorBind(wo, groundPlane);


   track_sphere = WO::New(yellowSphere, Vector(0.25f, 0.25f, 0.25f), MESH_SHADING_TYPE::mstFLAT);
   track_sphere->setPosition(Vector(20, 20, 100));
   worldLst->push_back(track_sphere);
   
   //createPhysicsModuleWayPoints();
}


void GLViewPhysicsModule::createPhysicsModuleWayPoints()
{
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 5000;
   params.useCamera = true;
   params.visible = true;
   WOWayPointSpherical* wayPt = WOWP1::New( params, 3 );
   wayPt->setPosition( Vector( 50, 0, 3 ) );
   worldLst->push_back( wayPt );
}
