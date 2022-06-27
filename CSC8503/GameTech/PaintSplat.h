#pragma once

#include "../CSC8503Common/GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class LevelManager;

		class PaintSplat : public GameObject {
		public:
			PaintSplat() : scale(6), growTimer(0), growDuration(0.3), lifetime(10), timeAlive(0) {};
			~PaintSplat() {};

			void Update(float dt) override {
				if (growTimer < growDuration) {
					growTimer += dt;
					transform.SetScale(scaleAxis * growTimer / growDuration);
				}
				timeAlive += dt;
				if (timeAlive > lifetime) {
					Remove();
				}
			}

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override {
				if (abs(point.normal.x) >= abs(point.normal.y) && abs(point.normal.x) >= abs(point.normal.z)) {
					scaleAxis.y = scale;
					scaleAxis.z = scale;
				}
				else if (abs(point.normal.y) >= abs(point.normal.x) && abs(point.normal.y) >= abs(point.normal.z)) {
					scaleAxis.x = scale;
					scaleAxis.z = scale;
				}
				else {
					scaleAxis.x = scale;
					scaleAxis.y = scale;
				}
			}

			void OnCollisionEnd(GameObject* otherObject) override {
				delete boundingVolume;
				boundingVolume = nullptr;
				delete physicsObject;
				physicsObject = nullptr;
			};

		protected:
			Vector3 scaleAxis;

			float scale;

			float growTimer;
			float growDuration;

			float lifetime;
			float timeAlive;
		};
	}
}