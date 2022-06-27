#pragma once

#include "../CSC8503Common/GameObject.h"
#include "ColourBlock.h"

namespace NCL {
	namespace CSC8503 {
		class LevelManager;

		class Agent : public GameObject {
		public:
			Agent(int agentID, LevelManager* l, const Vector3& spawnPosition, vector<ColourBlock*>& wall) : agentID(agentID), level(l), isRespawning(false), paintAmmo(12), spawnPos(spawnPosition) {
				targetWall = wall;
				name = "Agent"; 
				gunColourDuration = 2;
				gunColourTimer = 2;
				respawnDuration = 3.0;
				respawnTimer = 0;
				respawnFlag = true;
			}

			~Agent() {};

			virtual void Update(float dt) override;

			int GetID() const { return agentID; }

			int GetAmmo() const { return paintAmmo; }
			void SetAmmo(int a) { paintAmmo = a; }

			bool IsRespawning() const { return isRespawning; }
			void SetRespawning(bool s) { isRespawning = s; }

			Vector3 GetSpawnPosition() const { return spawnPos; }
			vector<ColourBlock*> GetTargetWall() { return targetWall; }

			int GetNumBlocksColoured() const;

			int GetNumBlocksRemaining() const;

			int GetHealth() const { return health; }

			void SetHealth(int newHealth) { health = newHealth; }

			void SetGunObj(GameObject* gun) { paintGun = gun; }

			void Explode();

		protected:

			int agentID;

			LevelManager* level;

			vector<ColourBlock*> targetWall;
			Vector3 spawnPos;

			int health = 100;

			int paintAmmo;
			bool isRespawning;

			float respawnDuration;
			float respawnTimer;
			bool respawnFlag;

			virtual void UpdateGun(float dt);
			GameObject* paintGun;
			Vector4 targetGunColour;
			Vector4 prevGunColour;
			float gunColourTimer;
			float gunColourDuration;

		};
	}
}