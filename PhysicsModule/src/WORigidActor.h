#pragma once
#include "WO.h"
#include "PxPhysicsAPI.h"

// Combination of a WO pointer and a RigidActor pointer
namespace Aftr {
	struct WORigidActor {
		public:
			physx::PxRigidActor* actor; // Px actor
			WO* wo; // Aftr object

			// New up a pointer
			static WORigidActor* New(WO* wo, physx::PxRigidActor* actor) {
				return new WORigidActor(wo, actor);
			}
		protected:
			// Regular constructor
			WORigidActor(WO* wo, physx::PxRigidActor* actor) {
				this->wo = wo;
				this->actor = actor;
			}
	};
}