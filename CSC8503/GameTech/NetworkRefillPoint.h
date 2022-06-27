#pragma once

#include "RefillPoint.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		class NetworkRefillPoint : public RefillPoint {
		public:
			NetworkRefillPoint(int id, NetworkedGame* g, bool server) : networkID(id), game(g), serverSide(server) {};
			~NetworkRefillPoint() {};

			void Update(float dt) override;

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override;

			int GetNetworkID() const { return networkID; }

			void SetActive(bool a) { a ? Activate() : Deactivate(-1); }

		protected:
			void Activate() override;
			void Deactivate(int collectedPlayerID);

			NetworkedGame* game;
			int networkID;
			bool serverSide;
		};
	}
}