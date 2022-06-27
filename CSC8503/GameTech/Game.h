#pragma once
#include "GameTechRenderer.h"
#include "LevelManager.h"
#include "Player.h"
#include "Opponent.h"
#include "ColourBlock.h"
#include "RefillPoint.h"

#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/SoundSystem.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "GameUI.h"

#include <map>

namespace NCL {
	namespace CSC8503 {
		class Game {
		public:
			Game();
			~Game();

			enum class State {
				PLAYING,
				PAUSED,
				WAITING,
				LOADING,
				MAIN_MENU
			};

			State GetState() const { return activeState; }
			virtual void ChangeState(State newState);
			virtual void UpdateGame(float dt);

			bool IsPlaying() const { return isPlaying; }

		protected:
			State activeState;
			State nextState;

			void UpdatePlayingState(float dt);
			void UpdatePausedState(float dt);
			virtual void UpdateWaitingState(float dt) {};
			void UpdateLoadingState(float dt);
			void UpdateMainMenuState(float dt);

			virtual void HandleUICommand();

			void InitCamera();
			void InitListener();
			void UpdateKeys();
			void InitUI();
			virtual void InitWorld();

			int GetBlocksRemaining(int agentID);
			virtual void DetermineWinners();
			bool gameOver;

			GameObject* SelectDebugObject();

			virtual Player* AddPlayerToWorld(int agentID, const Vector3& position, vector<ColourBlock*>& wall);
			Opponent* AddOpponentToWorld(int agentID, const Vector3& position, vector<RefillPoint*> refillPoints, vector<ColourBlock*>& wall);

			void AddCollisionLines();

			NavigationGrid* mapGrid;

			GameTechRenderer* renderer;
			PhysicsSystem* physics;
			GameWorld* world;
			LevelManager* levelManager;

			Player* player;
			vector<Agent*> winners;
			Agent* agents[4];

			map<int, vector<ColourBlock*>> colourWallMap;
			map<int, Vector3> spawnPoints;
			vector<RefillPoint*> refillPoints;

			bool useGravity;

			GameObject* audioListener;

			float gameTimer;

			int currentFrame;
			float frameTime;

			bool online;

			GameUI* gameUI;
			friend class GameUI;

			bool isPlaying = true;

		};
	}
}
