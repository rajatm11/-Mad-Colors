#include "Opponent.h"
#include "LevelManager.h"
#include "Player.h"
#include "ColourBlock.h"
#include "RefillPoint.h"

#include "../CSC8503Common/GameWorld.h"

#include "../CSC8503Common/NavigationGrid.h"

#include <algorithm>

NCL::CSC8503::Opponent::Opponent(int agentID, GameWorld* g, NavigationGrid* n, LevelManager* l, vector<ColourBlock*>& wall, const Vector3& startPoint, vector<RefillPoint*> fillPoints) : Agent(agentID, l, startPoint, wall) {
	name = "Opponent";

	world = g;
	navGrid = n;
	level = l;
	refillPoints = fillPoints;
	attackTarget = nullptr;

	transform.SetPosition(startPoint);
	transform.SetOrientation(Quaternion::LookRotation(l->GetEnvironmentCentre() - transform.GetPosition(), Vector3(0, 1, 0)));

	speed = 20.0f;
	turnSpeed = 0.08f;
	damping = 0.98f;
	distTolerance = 50.0f;
	attackDist = 3000.0f;
	attackMinRange = 500.0f;

	paintFireRate = (((float)rand() / RAND_MAX) * 0.2f) + 0.7f;
	paintFireTimer = 0.0f;
	paintShotForce = 10.0f;

	attackFireRate = (((float)rand() / RAND_MAX) * 0.2f) + 0.3f;
	attackFireTimer = 0.0f;

	decisionWaitDuration = 1.0f;
	decisionWaitTimer = 0.0f;

	respawnDuration = 3.0f;
	respawnTimer = 0;
	isRespawning = false;
	respawnFlag = true;

	stateMachine = new StateMachine();

	State* walkToWall = new State("Walk to Wall", [&](float dt)->void
		{
			this->TravelToObjective();
		}
	);

	State* walkToRefill = new State("Walk to Refill", [&](float dt)->void
		{
			this->FindAmmo();
		}
	);

	State* shootAtWall = new State("Shoot at Wall", [&](float dt)->void
		{
			this->ShootAtWall(dt);
		}
	);

	State* attackAgent = new State("Attack Agent", [&](float dt)->void
		{
			this->AttackOpponent(dt);
		}
	);

	State* respawning = new State("Respawning", [&](float dt)->void
		{
			this->Respawning(dt);
		}
	);

	State* walkToOpponentWall = new State("Walk to Opponent Wall", [&](float dt)->void
		{
			this->TravelToObjective();
		}
	);

	State* colourOpponentWall = new State("Colour Opponent Wall", [&](float dt)->void
		{
			this->ShootAtOpponentWall(dt);
		}
	);

	stateMachine->AddState(walkToWall);
	stateMachine->AddState(walkToRefill);
	stateMachine->AddState(shootAtWall);
	stateMachine->AddState(attackAgent);
	stateMachine->AddState(walkToOpponentWall);
	stateMachine->AddState(colourOpponentWall);
	stateMachine->AddState(respawning);

	// Ammo collected, return to paint wall
	stateMachine->AddTransition(new StateTransition(walkToRefill, walkToWall, [&]()->bool
		{
			if (paintAmmo > 0 && !WallIsPainted()) {
				currentPathTarget = spawnPos;
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Ammo collected, find highest scoring opponent and go to paint their wall
	stateMachine->AddTransition(new StateTransition(walkToRefill, walkToOpponentWall, [&]()->bool
		{
			if (paintAmmo > 0 && WallIsPainted()) {
				wallTarget = FindHighestScoringPlayer();
				currentPathTarget = wallTarget->GetSpawnPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Arrived at wall, begin shooting colour blocks
	stateMachine->AddTransition(new StateTransition(walkToWall, shootAtWall, [&]()->bool
		{
			if ((transform.GetPosition() - spawnPos).LengthSquared() < distTolerance) {
				return true;
			}
			return false;
		}
	));

	// Ran out of ammo, find a refill point and travel to it
	stateMachine->AddTransition(new StateTransition(shootAtWall, walkToRefill, [&]()->bool
		{
			if (paintAmmo <= 0) {
				currentPathTarget = FindNearestRefillPoint()->GetTransform().GetPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Finished painting own wall, seek out another player's wall to de-colour
	stateMachine->AddTransition(new StateTransition(shootAtWall, walkToOpponentWall, [&]()->bool
		{
			if (WallIsPainted()) {
				wallTarget = FindHighestScoringPlayer();
				currentPathTarget = wallTarget->GetSpawnPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Ran out of ammo, find a refill point and travel to it
	stateMachine->AddTransition(new StateTransition(colourOpponentWall, walkToRefill, [&]()->bool
		{
			if (paintAmmo <= 0) {
				currentPathTarget = FindNearestRefillPoint()->GetTransform().GetPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Arrived at opponent wall, begin shooting opponent colour blocks
	stateMachine->AddTransition(new StateTransition(walkToOpponentWall, colourOpponentWall, [&]()->bool
		{
			if ((transform.GetPosition() - wallTarget->GetSpawnPosition()).LengthSquared() < distTolerance) {
				return true;
			}
			return false;
		}
	));

	// Entirely de-coloured opponent wall or other wall needs targeting, find new opponent wall to de-colour
	stateMachine->AddTransition(new StateTransition(colourOpponentWall, walkToOpponentWall, [&]()->bool
		{
			if (decisionWaitTimer >= decisionWaitDuration && (rand() % targetWall.size()) * 0.5f > wallTarget->GetNumBlocksColoured()) {
				decisionWaitTimer = 0;
				wallTarget = FindHighestScoringPlayer();
				currentPathTarget = wallTarget->GetSpawnPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Own wall is being de-coloured, begin walking back to own wall
	stateMachine->AddTransition(new StateTransition(walkToOpponentWall, walkToWall, [&]()->bool
		{
			if (decisionWaitTimer >= decisionWaitDuration && !WallIsPainted() && (rand() % targetWall.size()) > GetNumBlocksColoured()) {
				decisionWaitTimer = 0;
				currentPathTarget = spawnPos;
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Own wall is being de-coloured, begin walking back to own wall
	stateMachine->AddTransition(new StateTransition(colourOpponentWall, walkToWall, [&]()->bool
		{
			if (decisionWaitTimer >= decisionWaitDuration && !WallIsPainted() && (rand() % targetWall.size()) > GetNumBlocksColoured()) {
				decisionWaitTimer = 0;
				currentPathTarget = spawnPos;
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Determine if any agent is a threat and begin attacking
	stateMachine->AddTransition(new StateTransition(walkToWall, attackAgent, [&]()->bool
		{
			attackTarget = ShouldAttack();
			return attackTarget;
		}
	));

	// Determine if any agent is a threat and begin attacking
	stateMachine->AddTransition(new StateTransition(shootAtWall, attackAgent, [&]()->bool
		{
			attackTarget = ShouldAttack();
			return attackTarget;
		}
	));

	// Determine if any agent is a threat and begin attacking
	stateMachine->AddTransition(new StateTransition(walkToOpponentWall, attackAgent, [&]()->bool
		{
			attackTarget = ShouldAttack();
			return attackTarget;
		}
	));

	// Determine if any agent is a threat and begin attacking
	stateMachine->AddTransition(new StateTransition(colourOpponentWall, attackAgent, [&]()->bool
		{
			attackTarget = ShouldAttack();
			return attackTarget;
		}
	));

	// Player is no longer a threat, return to previous activity
	stateMachine->AddTransition(new StateTransition(attackAgent, walkToWall, [&]()->bool
		{
			if (ShouldStopAttack() && (stateMachine->GetPreviousState()->GetName() == "Walk to Wall" || stateMachine->GetPreviousState()->GetName() == "Shoot at Wall")) {
				currentPathTarget = spawnPos;
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Player is no longer a threat, return to previous activity
	stateMachine->AddTransition(new StateTransition(attackAgent, walkToOpponentWall, [&]()->bool
		{
			if (ShouldStopAttack() && (stateMachine->GetPreviousState()->GetName() == "Walk to Opponent Wall" || stateMachine->GetPreviousState()->GetName() == "Colour Opponent Wall")) {
				wallTarget = FindHighestScoringPlayer();
				currentPathTarget = wallTarget->GetSpawnPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Health below zero, begin respawning
	stateMachine->AddTransition(new StateTransition(walkToRefill, respawning, [&]()->bool
		{
			if (isRespawning) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), transform.GetPosition(), 150);
			}
			return isRespawning;
		}
	));

	// Health below zero, begin respawning
	stateMachine->AddTransition(new StateTransition(walkToWall, respawning, [&]()->bool
		{
			if (isRespawning) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), transform.GetPosition(), 150);
			}
			return isRespawning;
		}
	));

	// Health below zero, begin respawning
	stateMachine->AddTransition(new StateTransition(shootAtWall, respawning, [&]()->bool
		{
			if (isRespawning) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), transform.GetPosition(), 150);
			}
			return isRespawning;
		}
	));

	// Health below zero, begin respawning
	stateMachine->AddTransition(new StateTransition(attackAgent, respawning, [&]()->bool
		{
			if (isRespawning) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), transform.GetPosition(), 150);
			}
			return isRespawning;
		}
	));

	// Health below zero, begin respawning
	stateMachine->AddTransition(new StateTransition(walkToOpponentWall, respawning, [&]()->bool
		{
			if (isRespawning) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), transform.GetPosition(), 150);
			}
			return isRespawning;
		}
	));

	// Health below zero, begin respawning
	stateMachine->AddTransition(new StateTransition(colourOpponentWall, respawning, [&]()->bool
		{
			if (isRespawning) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), transform.GetPosition(), 150);
			}
			return isRespawning;
		}
	));

	// Respawned, return to previous activity
	stateMachine->AddTransition(new StateTransition(respawning, walkToRefill, [&]()->bool
		{
			if (!isRespawning && (stateMachine->GetPreviousState()->GetName() == "Walk to Refill" || stateMachine->GetPreviousState()->GetName() == "Attack Agent")) {
				targetRefill = FindNearestRefillPoint();
				currentPathTarget = targetRefill->GetTransform().GetPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Respawned, return to previous activity
	stateMachine->AddTransition(new StateTransition(respawning, walkToWall, [&]()->bool
		{
			if (!isRespawning && (stateMachine->GetPreviousState()->GetName() == "Walk to Wall" || stateMachine->GetPreviousState()->GetName() == "Shoot at Wall")) {
				currentPathTarget = spawnPos;
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	// Respawned, return to previous activity
	stateMachine->AddTransition(new StateTransition(respawning, walkToOpponentWall, [&]()->bool
		{
			if (!isRespawning && (stateMachine->GetPreviousState()->GetName() == "Walk to Opponent Wall" || stateMachine->GetPreviousState()->GetName() == "Colour Opponent Wall")) {
				wallTarget = FindHighestScoringPlayer();
				currentPathTarget = wallTarget->GetSpawnPosition();
				FindPath(currentPathTarget);
				return true;
			}
			return false;
		}
	));

	targetRefill = FindNearestRefillPoint();
	currentPathTarget = targetRefill->GetTransform().GetPosition();
	FindPath(currentPathTarget);
}

void NCL::CSC8503::Opponent::Update(float dt) {

	if (Debug::GetAIActive()) {
		decisionWaitTimer += dt;

		stateMachine->Update(dt);

		if (decisionWaitTimer > decisionWaitDuration) {
			decisionWaitTimer = 0;
		}

		if (transform.GetPosition().y < -50 && !isRespawning) {
			isRespawning = true;
			respawnTimer = 0;
			GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		}
	}

	UpdateGun(dt);

	// Dampen opponent movement
	GetPhysicsObject()->SetLinearVelocity(GetPhysicsObject()->GetLinearVelocity() * Vector3(damping, 1, damping));

	// Avoid rotating opponent
	if (!isRespawning) {
		GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		transform.SetOrientation(Quaternion::EulerAnglesToQuaternion(0, transform.GetOrientation().ToEuler().y, 0));
	}

	if (Debug::IsActive())
		Debug::DrawLine(transform.GetPosition() + Vector3(0, 3, 0), transform.GetPosition() + transform.GetOrientation() * Vector3(0, 3, -3), Debug::CYAN);
}

void NCL::CSC8503::Opponent::OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) {
	if (otherObject->GetName() == "Refill Point") {
		paintAmmo = 12;
	}

	if (otherObject->GetName() == "Projectile") {
		health -= 20.0f;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("hurt.wav"), SoundPriority::SOUNDPRIORITY_HIGH, (((float)rand() / RAND_MAX) * 0.6f) + 0.7f);
		if (health <= 0) {
			isRespawning = true;
		}
	}
}

void NCL::CSC8503::Opponent::TravelToObjective() {
	FollowPath();
	LookAt(currentPathNode);
}

void NCL::CSC8503::Opponent::FindAmmo() {
	if (!targetRefill->IsActive()) {
		targetRefill = FindNearestRefillPoint();
		currentPathTarget = targetRefill->GetTransform().GetPosition();
		FindPath(currentPathTarget);
	}
	TravelToObjective();
}

void NCL::CSC8503::Opponent::ShootAtWall(float dt) {
	paintFireTimer += dt;

	if (!targetBlock || targetBlock->IsColoured()) {
		targetBlock = TargetNextBlock();
	}
	LookAt(targetBlock->GetTransform().GetPosition());
	if (paintFireTimer >= paintFireRate) {
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintshot.wav"), transform.GetPosition(), 150);
		ShootPaintAt(targetBlock->GetTransform().GetPosition() + Vector3(0, navGrid->GetNodeSize() * 0.5f, 0), true);
		paintFireTimer = 0;
		paintAmmo -= 1;
	}
}

void NCL::CSC8503::Opponent::AttackOpponent(float dt) {
	attackFireTimer += dt;

	Vector3 dirToPlayer = attackTarget->GetTransform().GetPosition() - transform.GetPosition();
	if (dirToPlayer.LengthSquared() >= attackMinRange) {
		GetPhysicsObject()->AddForce(dirToPlayer.Normalised() * speed);
	}
	LookAt(attackTarget->GetTransform().GetPosition());

	if (attackFireTimer >= attackFireRate) {
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintshot.wav"), transform.GetPosition(), 150);
		ShootPaintAt(attackTarget->GetTransform().GetPosition(), true);
		attackFireTimer = 0;
		paintAmmo -= 1;
	}
}

void NCL::CSC8503::Opponent::Respawning(float dt) {
	if (respawnFlag) {
		respawnFlag = false;
		Explode();
	}
	respawnTimer += dt;
	if (respawnTimer >= respawnDuration) {
		isRespawning = false;
		respawnTimer = 0;
		respawnFlag = true;
		Respawn();
	}
}

void NCL::CSC8503::Opponent::ShootAtOpponentWall(float dt) {
	paintFireTimer += dt;

	if (!targetBlock || !targetBlock->IsColoured()) {
		targetBlock = TargetOpponentBlock(wallTarget);
		if (!targetBlock) return;
	}
	LookAt(targetBlock->GetTransform().GetPosition());
	if (paintFireTimer >= paintFireRate) {
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintshot.wav"), transform.GetPosition(), 150);
		ShootPaintAt(targetBlock->GetTransform().GetPosition() + Vector3(0, navGrid->GetNodeSize() * 0.5f, 0), false);
		paintFireTimer = 0;
		paintAmmo -= 1;
	}
}

void NCL::CSC8503::Opponent::Respawn() {
	health = 100;
	transform.SetPosition(spawnPos);
	GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	paintAmmo = 0;
}

void NCL::CSC8503::Opponent::FindPath(const Vector3& targetPosition) {
	NavigationPath pathToTarget;
	bool pathFound = navGrid->FindPath(transform.GetPosition(), targetPosition, pathToTarget);

	currentPathNode = Vector3(0, 0, 0);

	if (pathFound) {
		currentPath.Clear();
		currentPath = pathToTarget;
		currentPath.PopWaypoint(currentPathNode);
		currentPathNode.x += navGrid->GetNodeSize() * 0.5f;
		currentPathNode.z += navGrid->GetNodeSize() * 0.5f;
		currentPathNode.y = transform.GetPosition().y;
	}
}

void NCL::CSC8503::Opponent::FollowPath() {
	Vector3 lineToNode = currentPathNode - transform.GetPosition();

	float squaredDist = abs(lineToNode.LengthSquared());

	if (Debug::IsActive()) {
		ShowPath();
	}

	if (squaredDist < distTolerance) {
		bool pathFinished = !currentPath.PopWaypoint(currentPathNode);
		if (!pathFinished) {
			if (currentPath.Size() > 0) {
				currentPathNode.x += navGrid->GetNodeSize() * 0.5f;
				currentPathNode.z += navGrid->GetNodeSize() * 0.5f;
			}
			currentPathNode.y = transform.GetPosition().y;
			lineToNode = currentPathNode - transform.GetPosition();
			squaredDist = abs(lineToNode.LengthSquared());
		}
	}

	GetPhysicsObject()->AddForce(lineToNode.Normalised() * speed);
}

void NCL::CSC8503::Opponent::ShowPath() {
	path.clear();
	NavigationPath p = currentPath;
	Vector3 pos;
	while (p.PopWaypoint(pos)) {
		if (p.Size() > 0) {
			pos.x += navGrid->GetNodeSize() * 0.5f;
			pos.z += navGrid->GetNodeSize() * 0.5f;
		}
		pos.y = transform.GetPosition().y + 0.5f;
		path.push_back(pos);
	}

	for (int i = 1; i < path.size(); ++i) {
		Vector3 a = path[i - 1];
		Vector3 b = path[i];

		Debug::DrawLine(a, b, Debug::BLUE);
	}
}

GameObject* NCL::CSC8503::Opponent::ShootRayAt(const Vector3& target) {
	Vector3 dir = (target - transform.GetPosition()).Normalised();
	Vector3 startPos = transform.GetPosition() + Vector3(0, 2, 0) + dir * 1.5f;
	Ray ray = Ray(startPos, dir);

	GameObject* closestObject;
	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true)) {
		closestObject = (GameObject*)closestCollision.node;
		return closestObject;
	}
	return nullptr;
}

void NCL::CSC8503::Opponent::ShootPaintAt(const Vector3& target, bool coloured) {
	Vector3 dir = (target - transform.GetPosition()).Normalised();
	Vector3 spawnPos = transform.GetPosition() + Vector3(0, 2.5f, 0) + transform.GetOrientation() * Vector3(0, 0, -4);
	if (Debug::IsActive()) {
		Debug::DrawLine(spawnPos, spawnPos + dir * 5, Debug::RED, 2);
	}
	GameObject* projectile = (GameObject*)level->AddProjectile(spawnPos, coloured);
	projectile->GetRenderObject()->SetColour(coloured ? Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1) : Vector4(1, 1, 1, 1));
	projectile->GetPhysicsObject()->ApplyLinearImpulse(dir * paintShotForce);
}

Agent* NCL::CSC8503::Opponent::ShouldAttack() {
	float shortestDist = FLT_MAX;
	Agent* attackCandidate = nullptr;
	for (Agent* o : opponents) {
		float distToOpponent = (transform.GetPosition() - o->GetTransform().GetPosition()).LengthSquared();
		if (distToOpponent < attackDist && paintAmmo > 0 && o->GetAmmo() > 0 && !o->IsRespawning() && CanSeeAgent(o) && distToOpponent < shortestDist) {
			shortestDist = distToOpponent;
			attackCandidate = o;
		}
	}
	return attackCandidate;
}

bool NCL::CSC8503::Opponent::ShouldStopAttack() {
	if (paintAmmo == 0) {
		return true;
	}
	if ((transform.GetPosition() - attackTarget->GetTransform().GetPosition()).LengthSquared() >= attackDist) {
		return true;
	}
	if (!CanSeeAgent(attackTarget)) {
		return true;
	}
	if (attackTarget->IsRespawning()) {
		return true;
	}
	return false;
}

RefillPoint* NCL::CSC8503::Opponent::FindNearestRefillPoint() {
	float shortestDist = FLT_MAX;
	RefillPoint* currentClosest = refillPoints[0];
	for (RefillPoint* p : refillPoints) {
		float distToPoint = (transform.GetPosition() - p->GetTransform().GetPosition()).LengthSquared();
		if (distToPoint < shortestDist && p->IsActive()) {
			shortestDist = distToPoint;
			currentClosest = p;
		}
	}
	return currentClosest;
}

bool NCL::CSC8503::Opponent::WallIsPainted() {
	for (ColourBlock* block : targetWall) {
		if (!block->IsColoured()) {
			return false;
		}
	}
	targetBlock = nullptr;
	return true;
}

Agent* NCL::CSC8503::Opponent::FindHighestScoringPlayer() {
	// Shuffle to ensure same player is not picked repeatedly
	vector<Agent*> agents = { opponents[0], opponents[1], opponents[2] };
	std::random_shuffle(agents.begin(), agents.end());

	int bestScore = 0;
	Agent* highScoreCandidate = agents[0];
	for (Agent* o : agents) {
		int numBlocksColoured = o->GetNumBlocksColoured();
		if (numBlocksColoured > bestScore) {
			highScoreCandidate = o;
			bestScore = numBlocksColoured;
		}
	}
	return highScoreCandidate;
}

bool NCL::CSC8503::Opponent::CanSeeAgent(Agent* opponent) {
	GameObject* hit = ShootRayAt(opponent->GetTransform().GetPosition());
	return hit && (hit->GetName() == "Player" || hit->GetName() == "Opponent");
}

ColourBlock* NCL::CSC8503::Opponent::TargetNextBlock() {
	for (ColourBlock* block : targetWall) {
		if (!block->IsColoured()) {
			return block;
		}
	}
}

ColourBlock* NCL::CSC8503::Opponent::TargetOpponentBlock(Agent* o) {
	for (ColourBlock* block : o->GetTargetWall()) {
		if (block->IsColoured()) {
			return block;
		}
	}
	return nullptr;
}

void NCL::CSC8503::Opponent::LookAt(const Vector3& target) {
	Vector3 viewVector = (transform.GetPosition() - target).Normalised();
	Quaternion targetRot = Quaternion::LookRotation(viewVector, Vector3(0, 1, 0));
	transform.SetOrientation(Quaternion::Lerp(transform.GetOrientation(), targetRot, turnSpeed));
}