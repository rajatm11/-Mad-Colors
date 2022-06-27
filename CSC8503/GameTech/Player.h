#pragma once

#include "Agent.h"
#include "../../Common/Camera.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class LevelManager;

		class Player : public Agent {
		public:
			Player(int agentID, Camera* cam, Vector3 offset, GameWorld* gameWorld, LevelManager* l, vector<ColourBlock*>& wall, const Vector3& startPoint);
			~Player() {};

			void Update(float dt) override;

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override;

			void SetCameraAttached(bool a, bool reset = false) {
				cameraAttached = a;
				controlling = a;
				camera->SetIsFree(!a);
				if (reset) {
					camera->SetPitch(0);
					camera->SetYaw(0);
				}
			}

			bool GetCameraAttached() const {
				return cameraAttached;
			}

			void SetCameraActive(bool a) {
				cameraActive = a;
				camera->SetIgnoreInput(!a);
			}

			bool GetCameraActive() const {
				return cameraActive;
			}


			int GetPaintAmmo() const {
				return paintAmmo;
			}

			void SetControlling(bool c) {
				controlling = c;
				camera->SetIgnoreInput(!c);
			}

			bool IsControlling() const {
				return controlling;
			}

			float GetPitch() const {
				return camera->GetPitch();
			}

			float GetYaw() const {
				return camera->GetYaw();
			}

			int GetFiringInfo() const {
				return firingInfo;
			}

		protected:

			void UpdateMouse();
			void UpdateKeys();
			void UpdateGun(float dt) override;

			GameObject* ShootRay();
			void ShootPaint(bool coloured);
			void Respawn();

			float yaw;

			bool thirdPerson;

			float paintShotRate;
			float paintShotTimer;
			float paintShotForce;

			bool grounded;
			float jumpCooldownDuration;
			float jumpCooldownTimer;

			float speed;
			float jumpForce;
			float damping;

			bool controlling;
			bool cameraActive;

			int firingInfo = -1; // -1 no fire, 0 is left fire, 1 is right fire

			// Camera properties
			bool cameraAttached = false;
			Vector3 fpCamOffset;
			Vector3 tpCamOffset;
			Vector3 camOffset;
			Camera* camera;

			GameWorld* world;
		};
	}
}