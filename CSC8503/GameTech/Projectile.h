#pragma once

#include "../CSC8503Common/GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class LevelManager;

		class Projectile : public GameObject {
		public:
			Projectile(GameWorld* g, LevelManager* level, bool colourProjectile);
			~Projectile() {};

			void Update(float dt) override;

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override;

			bool IsColoured() const { return colourProjectile; }


		protected:
			GameWorld* world;
			LevelManager* level;

			float lifetime;
			float timeAlive;

			bool colourProjectile;
		};
	}
}
