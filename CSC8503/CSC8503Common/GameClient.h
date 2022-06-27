#pragma once
#include "NetworkBase.h"
#include <stdint.h>
#include <thread>
#include <atomic>

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class GameClient : public NetworkBase {
		public:
			GameClient();
			~GameClient();

			bool Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum);
			void Disconnect();

			void SendPacket(GamePacket&  payload);

			void UpdateClient();

			ENetPeer* GetClientPeer() { return netPeer; }

			void SetClientID(int id) { clientID = id; }
			int GetClientID() const { return clientID; }

		protected:	
			//void ThreadedUpdate();

			ENetPeer*	netPeer;
			int			clientID;
			//std::atomic<bool>	threadAlive;
			//std::thread			updateThread;
		};
	}
}

