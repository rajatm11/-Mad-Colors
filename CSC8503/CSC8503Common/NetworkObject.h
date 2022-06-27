#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
namespace NCL {
	namespace CSC8503 {

		struct FullPacket : public GamePacket {
			int		playerID = -1;
			int		objectID = -1;
			NetworkState fullState;

			FullPacket() {
				type = Full_State;
				size = sizeof(FullPacket) - sizeof(GamePacket);
			}
		};

		struct DeltaPacket : public GamePacket {
			int		fullID		= -1;
			int		objectID	= -1;
			char	pos[3];
			char	orientation[4];

			DeltaPacket() {
				type = Delta_State;
				size = sizeof(DeltaPacket) - sizeof(GamePacket);
			}
		};

		struct ClientPacket : public GamePacket {
			int		playerID = -1;
			int		prevPacketID = -1;
			float	position[3];
			float	orientation[4];
			float	pitch;
			float	yaw;
			int		firingInfo = -1;

			ClientPacket() {
				type = Received_State;
				size = sizeof(ClientPacket);
			}
		};

		struct ProjectilePacket : public GamePacket {
			int		objectID = -1;
			float	position[3];
			float	colour[4];

			ProjectilePacket() {
				type = New_Projectile;
				size = sizeof(ProjectilePacket);
			}
		};

		struct DestroyProjectilePacket : public GamePacket {
			int		objectID = -1;
			float	position[3];
			float	normal[3];
			float	penetration;
			int		hitPlayerID = -1;
			bool	paintSplat = false;

			DestroyProjectilePacket() {
				type = Destroy_Projectile;
				size = sizeof(DestroyProjectilePacket);
			}
		};

		struct ColourBlockPacket : public GamePacket {
			int		objectID = -1;
			bool	coloured;
			float	colour[4];

			ColourBlockPacket() {
				type = ColourBlockUpdate;
				size = sizeof(ColourBlockPacket);
			}
		};

		struct RefillPointPacket : public GamePacket {
			int		objectID = -1;
			bool	available;
			int		collectPlayerID = -1;

			RefillPointPacket() {
				type = RefillPointUpdate;
				size = sizeof(RefillPointPacket);
			}
		};

		struct GameTimerPacket : public GamePacket {
			float gameTime;

			GameTimerPacket(float gt) {
				type = Game_Timer;
				gameTime = gt;
				size = sizeof(GameTimerPacket);
			}
		};

		class NetworkObject		{
		public:
			NetworkObject(GameObject& o, int id);
			virtual ~NetworkObject();

			//Called by clients
			virtual bool ReadPacket(GamePacket& p);
			//Called by servers
			virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

			void UpdateStateHistory(int minID);

			void SetNetworkID(int id) {
				networkID = id;
			}

			int GetNetworkID() const {
				return networkID;
			}

			GameObject* GetGameObject() const {
				return &object;
			}

		protected:

			NetworkState& GetLatestNetworkState();

			bool GetNetworkState(int frameID, NetworkState& state);

			virtual bool ReadDeltaPacket(DeltaPacket &p);
			virtual bool ReadFullPacket(FullPacket &p);

			virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
			virtual bool WriteFullPacket(GamePacket**p);

			GameObject& object;

			NetworkState lastFullState;

			std::vector<NetworkState> stateHistory;

			int deltaErrors;
			int fullErrors;

			int networkID;
		};
	}
}

