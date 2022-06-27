#pragma once

#include "../CSC8503Common/GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class RefillPoint : public GameObject {
		public:
			RefillPoint();
			~RefillPoint() {};

			void Update(float dt) override;

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override;

			bool IsActive() const { return isActive; }

		protected:
			virtual void Activate();
			void Deactivate();

			bool isActive;

			float cooldownTimer;
			float cooldownDuration;

			float rotationSpeed;

		};
	}
}