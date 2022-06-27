#pragma once
#include "Game.h"
#include "../CSC8503Common/NetworkObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		class NetworkProjectile;
		class NetworkLevelManager;

		class NetworkedGame : public Game, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;
			void ChangeState(State newState) override;

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);
			void OnProjectileDestroyed(int objectID, Vector3 position = Vector3(0, 0, 0), Vector3 normal = Vector3(0, 0, 0), float penetration = 0, bool paintSplat = false, int playerID = -1);
			void OnWallBlockColoured(int objectID, Vector4 colour, bool coloured);
			void OnRefillPointStateChanged(int objectID, bool available, int playerID = -1);

			int GetNextObjectIDAndIncrement();
			void AddNetworkObject(NetworkObject* o, int networkID);

		protected:
			void UpdateWaitingState(float dt) override;

			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			bool ConnectClient(string& fullIP);
			void ConnectPlayer();
			void UpdatePlayer(ClientPacket* packet);

			void InitialiseLocalPlayer(int playerID);
			void InitialiseNetworkPlayer(int playerID, int objectID);
			void UpdateNetworkPlayer(FullPacket* packet);

			void UpdateObjectState(FullPacket* packet);

			void HandleUICommand() override;

			void InitWorld() override;
			void DetermineWinners() override;

			void Reset();

			Player* AddPlayerToWorld(int agentID, int objectID, const Vector3& position, vector<ColourBlock*>& wall);
			Agent* AddAgentToWorld(int agentID, int objectID, const Vector3& position, vector<ColourBlock*>& wall);

			void FireProjectile(ClientPacket* packet);
			void FireProjectile(ProjectilePacket* packet);

			void BroadcastSnapshot(bool deltaFrame);

			GameServer* thisServer;
			GameClient* thisClient;

			std::map<int, NetworkObject*> networkObjects;
			int nextObjectID = 0;

			std::map<int, Agent*> serverPlayers;
			int nextPlayerID = 0;

			Player* localPlayer;
			int playerPacketID;

			float paintShotForce = 10;

			friend class GameUI;
		};
	}
}