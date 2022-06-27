#pragma once

#include "Agent.h"
#include "../CSC8503Common/NavigationPath.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class NavigationGrid;
		class LevelManager;
		class ColourBlock;
		class RefillPoint;

		class Opponent : public Agent {
		public:
			Opponent(int agentID, GameWorld* gameWorld, NavigationGrid* grid, LevelManager* level, vector<ColourBlock*> &wall, const Vector3& startPoint, vector<RefillPoint*> refillPoints);
			~Opponent() {};

			void Update(float dt) override;

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override;

			void SetOpponents(vector<Agent*>& o) { opponents = o; }

			string GetActiveStateName() const { return stateMachine->GetActiveState()->GetName(); }
			string GetPrevStateName() const { return stateMachine->GetPreviousState()->GetName(); }

		protected:

			void TravelToObjective();
			void FindAmmo();
			void ShootAtWall(float dt);
			void AttackOpponent(float dt);
			void Respawning(float dt);
			void ShootAtOpponentWall(float dt);

			void Respawn();

			vector<RefillPoint*> refillPoints;
			RefillPoint* targetRefill;

			Vector3 currentPathTarget;

			float distTolerance;
			float attackDist;

			ColourBlock* targetBlock;

			float paintFireRate;
			float paintFireTimer;
			float paintShotForce;

			float attackFireRate;
			float attackFireTimer;
			float attackMinRange;

			float decisionWaitDuration;
			float decisionWaitTimer;

			NavigationGrid* navGrid;
			NavigationPath currentPath;
			Vector3 currentPathNode;
			vector<Vector3> path;

			void FindPath(const Vector3& targetPosition);
			void FollowPath();
			void ShowPath();
			void LookAt(const Vector3& target);
			GameObject* ShootRayAt(const Vector3& target);
			void ShootPaintAt(const Vector3& target, bool coloured);
			bool CanSeeAgent(Agent* opponent);
			Agent* ShouldAttack();
			bool ShouldStopAttack();
			RefillPoint* FindNearestRefillPoint();
			bool WallIsPainted();
			Agent* FindHighestScoringPlayer();

			ColourBlock* TargetNextBlock();
			ColourBlock* TargetOpponentBlock(Agent* o);

			StateMachine* stateMachine;

			float speed;
			float damping;
			float turnSpeed;

			vector<Agent*> opponents;
			Agent* attackTarget;
			Agent* wallTarget;

			GameWorld* world;
		};
	}
}
