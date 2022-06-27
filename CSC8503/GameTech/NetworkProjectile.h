#pragma once

#include "Projectile.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		class NetworkProjectile : public Projectile {
		public:
			NetworkProjectile(int id, NetworkedGame* g, LevelManager* level, GameWorld* gw, bool colourProjectile) : networkID(id), game(g), Projectile(gw, level, colourProjectile) {};
			~NetworkProjectile() {};

			void Update(float dt) override;

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override;

			int GetNetworkID() const { return networkID; }

		protected:
			NetworkedGame* game;
			int networkID;
		};
	}
}