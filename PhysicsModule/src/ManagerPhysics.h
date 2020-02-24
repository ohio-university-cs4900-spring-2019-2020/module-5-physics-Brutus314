#pragma once

#include "PxPhysicsAPI.h"
#include "WO.h"

namespace Aftr
{
	// This manager is meant to be a singleton container for the PhysX Scene class
	class ManagerPhysics {
		protected:
			static physx::PxDefaultAllocator gAllocator;
			static physx::PxDefaultErrorCallback gErrorCallback;
			static physx::PxFoundation* gFoundation;
			static physx::PxSceneDesc gSceneDesc;
			static physx::PxDefaultCpuDispatcher* gCpuDispatcher;
			
		public:
			// Initialize the engine
			static void init();
			// Drops the engine
			static void shutdown();
			// Add an object to the scene and bind it to a WO
			static void addActorBind(void* pointer, physx::PxActor* actor);

			static physx::PxPhysics* gPhysics;
			static physx::PxScene* scene;
	};
}