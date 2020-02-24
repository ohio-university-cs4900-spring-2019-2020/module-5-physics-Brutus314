#include <iostream>
#include "ManagerPhysics.h"
#include "AftrGlobals.h"

using namespace Aftr;
using namespace physx;

// These line are required to use as a singleton
PxDefaultAllocator ManagerPhysics::gAllocator;
PxDefaultErrorCallback ManagerPhysics::gErrorCallback;
PxFoundation* ManagerPhysics::gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
PxPhysics* ManagerPhysics::gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true);
PxSceneDesc ManagerPhysics::gSceneDesc(gPhysics->getTolerancesScale());
PxDefaultCpuDispatcher* ManagerPhysics::gCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
PxScene* ManagerPhysics::scene;

void ManagerPhysics::init() {
	// Initialize the engine
	gSceneDesc.cpuDispatcher = gCpuDispatcher;
	gSceneDesc.filterShader = PxDefaultSimulationFilterShader;
	scene = gPhysics->createScene(gSceneDesc);
	scene->setFlag(PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);
	// If it didn't start, display an error message
	if (!scene) {
		std::cout << "Unable to create PhysX engine" << std::endl;
	}
	else {
		scene->setGravity(PxVec3(0, 0, -1 * GRAVITY));
	}
}

void ManagerPhysics::shutdown() {
	// Drop the engine, opposite of init
	if (gFoundation != nullptr) gFoundation->release();
	if (gPhysics != nullptr) gPhysics->release();
	if (scene != nullptr) scene->release();
}

void ManagerPhysics::addActorBind(void* pointer, physx::PxActor* actor) {
	actor->userData = pointer; // Bind this object in the physx world to something
	scene->addActor(*actor);
}