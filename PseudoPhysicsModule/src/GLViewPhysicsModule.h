#pragma once

#include "GLView.h"
#include "NetMessengerClient.h"

#include <vector>

namespace Aftr
{
   class Camera;

/**
   \class GLViewPhysicsModule
   \author Scott Nykl 
   \brief A child of an abstract GLView. This class is the top-most manager of the module.

   Read \see GLView for important constructor and init information.

   \see GLView

    \{
*/

class GLViewPhysicsModule : public GLView
{
public:
   static GLViewPhysicsModule* New( const std::vector< std::string >& outArgs );
   virtual ~GLViewPhysicsModule();
   virtual void updateWorld(); ///< Called once per frame
   virtual void loadMap(); ///< Called once at startup to build this module's scene
   virtual void createPhysicsModuleWayPoints();
   virtual void onResizeWindow( GLsizei width, GLsizei height );
   virtual void onMouseDown( const SDL_MouseButtonEvent& e );
   virtual void onMouseUp( const SDL_MouseButtonEvent& e );
   virtual void onMouseMove( const SDL_MouseMotionEvent& e );
   virtual void onKeyDown( const SDL_KeyboardEvent& key );
   virtual void onKeyUp( const SDL_KeyboardEvent& key );

   WO* track_sphere;
   std::vector<WO*> placed_cubes; // Store the cubes placed. Doesn't need a map since we obtain index

protected:
   GLViewPhysicsModule( const std::vector< std::string >& args );
   virtual void onCreate();  

   NetMessengerClient* client;
};

/** \} */

} //namespace Aftr
