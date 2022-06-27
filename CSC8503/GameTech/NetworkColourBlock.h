#pragma once

#include "ColourBlock.h"
#include "NetworkProjectile.h"
#include "NetworkedGame.h"

namespace NCL {
	namespace CSC8503 {

		class NetworkColourBlock : public ColourBlock {
		public:
			NetworkColourBlock(int id, NetworkedGame* g) : networkID(id), game(g) {};
			~NetworkColourBlock() {};

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override {
				string msg = "Worked";
				Debug::PrintToConsole(msg);
				if (otherObject->GetName() == "Projectile") {
					NetworkProjectile* p = static_cast<NetworkProjectile*>(otherObject);
					coloured = p->IsColoured();
					game->OnWallBlockColoured(networkID, p->GetRenderObject()->GetColour(), coloured);
				}
			};

			void SetColoured(bool c) {
				coloured = c;
			}

			int GetNetworkID() const { return networkID; }

		protected:
			NetworkedGame* game;
			int networkID;
		};
	}
}