#include "Game.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/Assets.h"
#include "../GameTech/ColliderLineObj.h"

#include "../../Common/Quaternion.h"
#include <typeinfo>

using namespace NCL;
using namespace CSC8503;

Game::Game() {
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);
	levelManager = new LevelManager(this, *world);	

	useGravity = true;
	online = false;

	Debug::Initialise();
	Debug::SetRenderer(renderer);

	InitUI();

	world->AddCollisionIgnore(CollisionLayer::RAY, CollisionLayer::IGNORE_RAYCAST);
	world->AddCollisionIgnore(CollisionLayer::IGNORE_DEFAULT, CollisionLayer::DEFAULT);

	levelManager->InitialiseAssets();
	ChangeState(State::LOADING);
}

Game::~Game() {
	Debug::Destroy();
	delete physics;
	delete renderer;
	delete world;
	delete levelManager;
}

void Game::UpdateGame(float dt) {
	switch (activeState) {
	case State::PLAYING:
		UpdatePlayingState(dt);
		break;
	case State::PAUSED:
		UpdatePausedState(dt);
		break;
	case State::WAITING:
		UpdateWaitingState(dt);
		break;
	case State::LOADING:
		UpdateLoadingState(dt);
		break;
	case State::MAIN_MENU:
		UpdateMainMenuState(dt);
		break;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		Debug::SetFullScreen(!Debug::GetFullScreen());
		Window::GetWindow()->SetFullScreen(Debug::GetFullScreen());
	}

	if (!levelManager->IsLoadingAssets()) {
		world->UpdateWorld(dt);

		SoundSystem::GetSoundSystem()->Update(dt);
		audioListener->GetTransform().SetPosition(world->GetMainCamera()->GetPosition());

		renderer->Update(dt);

		Debug::GetInstance()->UpdateInfo(dt);

		gameUI->UpdateUI(dt);

		Debug::FlushRenderables(dt);
		renderer->Render(currentFrame);

		world->Prune();
	}
}

void NCL::CSC8503::Game::ChangeState(State newState) {

	SoundSystem::GetSoundSystem()->SetMasterVolume(1);

	if (newState == State::MAIN_MENU) {
		world->ClearAndErase();
		physics->Clear();
		world->GetMainCamera()->SetYaw(105.0f);
		world->GetMainCamera()->SetPitch(5.0f);
		InitListener();
		gameTimer = 90;
		gameUI->ResetPlayers();
		gameUI->SetValid(true);
		Window::GetWindow()->ShowOSPointer(true);
		Debug::Reset();
		levelManager->AddSoundEmitterToWorld(Vector3(1000, 1000, 1000), "menumusic.wav", SoundPriority::SOUNDPRIORITY_HIGH, 0.15f, true, true);
	}

	if (newState == State::PAUSED) {
		if (!gameOver) {
			SoundSystem::GetSoundSystem()->SetMasterVolume(0);
		}
		if (!player->IsRespawning()) {
			player->SetCameraAttached(false, false);
		}
		Window::GetWindow()->ShowOSPointer(true);
	}

	if (activeState == State::MAIN_MENU && newState == State::PLAYING) {
		InitCamera();
		InitWorld();
		gameOver = false;
	}

	if (newState == State::PLAYING) {
		player->SetCameraAttached(true, false);
		Window::GetWindow()->ShowOSPointer(false);
	}

	activeState = newState;
}

void NCL::CSC8503::Game::UpdatePlayingState(float dt) {
	gameTimer -= dt;

	UpdateKeys();

	renderer->DrawString(".", Vector2(50, 50), Debug::WHITE, 12.5f);

	if (Debug::IsActive() && Window::GetMouse()->ButtonPressed(NCL::MouseButtons::LEFT)) {
		Debug::SetSelectedObject(SelectDebugObject());
	}
	physics->Update(dt);

	if (!player || !player->GetCameraAttached() || Debug::GetFreeCam()) {
		world->GetMainCamera()->UpdateCamera(dt);
		if (player) player->SetCameraAttached(!Debug::GetFreeCam());
	}

	// 8508 animation
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % levelManager->GetAnimation("StepForward")->GetFrameCount();
		frameTime += 1.0f / levelManager->GetAnimation("StepForward")->GetFrameRate();
	}

	if (gameTimer <= 0) {
		gameOver = true;
		DetermineWinners();
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("end.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
		ChangeState(State::PAUSED);
	}

	if (!online && Window::GetWindow()->GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
		ChangeState(State::PAUSED);
	}
}

void NCL::CSC8503::Game::UpdatePausedState(float dt) {
	HandleUICommand();
}

void NCL::CSC8503::Game::UpdateLoadingState(float dt) {
	int percentComplete = levelManager->LoadNextAsset();
	//renderer->DrawString(to_string(percentComplete) + "% Complete", Vector2(50, 50), Debug::CYAN, 30);
	renderer->Update(dt);
	gameUI->SetAmountLoaded(percentComplete);
	gameUI->UpdateUI(dt);
	renderer->Render(currentFrame);
}

void NCL::CSC8503::Game::UpdateMainMenuState(float dt) {
	world->GetMainCamera()->SetYaw(world->GetMainCamera()->GetYaw() + 0.025f);
	HandleUICommand();
}

void NCL::CSC8503::Game::HandleUICommand() {
	GameUI::Command command = gameUI->GetFrameCommand();

	if (activeState == State::PAUSED) {
		switch (command) {
		case GameUI::Command::CONTINUE:
			ChangeState(State::PLAYING);
			break;
		case GameUI::Command::EXIT_TO_MENU:
			ChangeState(State::MAIN_MENU);
			break;
		}
	}
	else if (activeState == State::MAIN_MENU) {
		switch (command) {
		case GameUI::Command::START_SINGLEPLAYER:
			ChangeState(State::PLAYING);
			break;
		case GameUI::Command::EXIT_GAME:
			isPlaying = false;
			break;
		}
	}
}

void Game::UpdateKeys() {
	if (!online && Window::GetKeyboard()->KeyPressed(KeyboardKeys::TAB)) {
		Debug::GetInstance()->SetActive(!Debug::GetInstance()->IsActive());
		Window::GetWindow()->ShowOSPointer(Debug::IsActive());
	}

	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}
}

void NCL::CSC8503::Game::InitUI() {
	gameUI = new GameUI(this);
	renderer->SetUI(gameUI);
	gameUI->SetValid(true);
}

int NCL::CSC8503::Game::GetBlocksRemaining(int agentID) {
	vector<ColourBlock*> wall = colourWallMap[agentID];
	int blocksRemaining = 0;
	for (ColourBlock* block : wall) {
		if (!block->IsColoured()) {
			blocksRemaining++;
		}
	}
	return blocksRemaining;
}

void NCL::CSC8503::Game::DetermineWinners() {
	int bestScore = 0;
	for (Agent* o : agents) {
		int numBlocksColoured = o->GetNumBlocksColoured();
		if (numBlocksColoured > bestScore) {
			bestScore = numBlocksColoured;
			winners.clear();
			winners.push_back(o);
		}
		else if (numBlocksColoured == bestScore) {
			winners.push_back(o);
		}
	}
}

void Game::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
}

void NCL::CSC8503::Game::InitListener() {
	audioListener = new GameObject("Listener");
	SoundSystem::GetSoundSystem()->SetListener(audioListener);
	world->AddGameObject(audioListener);
}

void Game::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitListener();

	vector<ColourBlock*> colourWalls[4];

	levelManager->LoadEnvironment("LevelData.txt", colourWalls);
	mapGrid = new NavigationGrid("LevelLayout.txt");
	refillPoints.push_back(levelManager->AddRefillPoint(levelManager->GetEnvironmentCentre() - Vector3(0, 2, 0), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(levelManager->GetEnvironmentCentre() + Vector3(50, -2, 0), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(levelManager->GetEnvironmentCentre() + Vector3(0, -2, 50), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(levelManager->GetEnvironmentCentre() - Vector3(50, 2, 0), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(levelManager->GetEnvironmentCentre() - Vector3(0, 2, 50), 2.5f));

	spawnPoints[0] = Vector3(40, 0, 40);
	spawnPoints[1] = Vector3(160, 0, 40);
	spawnPoints[2] = Vector3(40, 0, 160);
	spawnPoints[3] = Vector3(160, 0, 160);

	agents[0] = AddPlayerToWorld(0, spawnPoints[0], colourWalls[0]);
	agents[1] = AddOpponentToWorld(1, spawnPoints[1], refillPoints, colourWalls[1]);
	agents[2] = AddOpponentToWorld(2, spawnPoints[2], refillPoints, colourWalls[2]);
	agents[3] = AddOpponentToWorld(3, spawnPoints[3], refillPoints, colourWalls[3]);

	colourWallMap[0] = colourWalls[0];
	colourWallMap[1] = colourWalls[1];
	colourWallMap[2] = colourWalls[2];
	colourWallMap[3] = colourWalls[3];

	vector<Agent*> pOneOpponents{ agents[1], agents[2], agents[3] };
	vector<Agent*> pTwoOpponents{ agents[0], agents[2], agents[3] };
	vector<Agent*> pThreeOpponents{ agents[0], agents[1], agents[3] };
	vector<Agent*> pFourOpponents{ agents[0], agents[1], agents[2] };

	vector<Agent*> opponentLists[4];
	opponentLists[0] = pOneOpponents;
	opponentLists[1] = pTwoOpponents;
	opponentLists[2] = pThreeOpponents;
	opponentLists[3] = pFourOpponents;

	for (int i = 0; i < 4; ++i) {
		if (agents[i]->GetName() == "Opponent") {
			static_cast<Opponent*>(agents[i])->SetOpponents(opponentLists[i]);
		}
	}

	gameUI->SetPlayer(0, agents[0]);
	gameUI->SetPlayer(1, agents[1]);
	gameUI->SetPlayer(2, agents[2]);
	gameUI->SetPlayer(3, agents[3]);

	//for debug mode
	AddCollisionLines();

	world->BuildStaticTree();
}

GameObject* NCL::CSC8503::Game::SelectDebugObject() {
	GameObject* selectedObject = nullptr;
	Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true)) {
		selectedObject = (GameObject*)closestCollision.node;
	}
	return selectedObject;
}

Player* NCL::CSC8503::Game::AddPlayerToWorld(int agentID, const Vector3& position, vector<ColourBlock*>& wall)
{
	float halfHeight = 2;
	float radius = 1;
	float inverseMass = 5;

	player = new Player(agentID, world->GetMainCamera(), Vector3(0, 3.5f, 0), world, levelManager, wall, position);

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius, Vector3(0, 2, 0));
	player->SetBoundingVolume((CollisionVolume*)volume);

	player->GetTransform()
		.SetScale(Vector3(radius * -2, halfHeight, radius * 1.5))
		.SetPosition(position);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), levelManager->GetMesh("Male_Guard"), levelManager->GetDefaultTexture(), levelManager->GetShader("guard")));
	player->GetRenderObject()->SetFlag(1);
	player->GetRenderObject()->SetTextures(levelManager->GuardTextures);
	player->GetRenderObject()->SetAnimation(levelManager->GetAnimation("StepForward"));

	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));
	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitCubeInertia();
	player->GetPhysicsObject()->SetElasticity(0.1f);

	GameObject* gun = new GameObject();
	gun->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2) * Vector3(2, 2, 2))
		.SetPosition(position);

	gun->SetRenderObject(new RenderObject(&gun->GetTransform(), levelManager->GetMesh("THW_Ranged_SMGSoldier"), levelManager->GetTexture("gun"), levelManager->GetDefaultShader()));
	player->SetGunObj(gun);

	world->AddGameObject(player);
	world->AddGameObject(gun);

	return player;
}

Opponent* NCL::CSC8503::Game::AddOpponentToWorld(int agentID, const Vector3& position, vector<RefillPoint*> refillPoints, vector<ColourBlock*> &wall) {
	
	float halfHeight = 2;
	float radius = 1;
	float inverseMass = 5;

	Opponent* opponent = new Opponent(agentID, world, mapGrid, levelManager, wall, position, refillPoints);

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius, Vector3(0, 2, 0));
	opponent->SetBoundingVolume((CollisionVolume*)volume);

	opponent->GetTransform()
		.SetScale(Vector3(radius * -2, halfHeight, radius * 2));

	opponent->SetRenderObject(new RenderObject(&opponent->GetTransform(), levelManager->GetMesh("Male_Guard"), levelManager->GetDefaultTexture(), levelManager->GetShader("guard")));
	opponent->GetRenderObject()->SetFlag(1);
	opponent->GetRenderObject()->SetTextures(levelManager->GuardTextures);
	opponent->GetRenderObject()->SetAnimation(levelManager->GetAnimation("StepForward"));
	
	opponent->SetPhysicsObject(new PhysicsObject(&opponent->GetTransform(), opponent->GetBoundingVolume()));
	opponent->GetPhysicsObject()->SetInverseMass(inverseMass);
	opponent->GetPhysicsObject()->InitCubeInertia();
	opponent->GetPhysicsObject()->SetElasticity(0.1f);

	GameObject* gun = new GameObject();
	gun->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2) * Vector3(2, 2, 2))
		.SetPosition(position);

	gun->SetRenderObject(new RenderObject(&gun->GetTransform(), levelManager->GetMesh("THW_Ranged_SMGSoldier"), levelManager->GetTexture("gun"), levelManager->GetDefaultShader()));
	opponent->SetGunObj(gun);

	world->AddGameObject(gun);
	world->AddGameObject(opponent);

	return opponent;
}

void NCL::CSC8503::Game::AddCollisionLines() {
	for (auto obj : world->GetGameObjects()) {
		if (obj->GetBoundingVolume()) {
			ColliderLine* collisionLineObj = new ColliderLine(obj, levelManager);
			collisionLineObj->GetRenderObject()->SetFlag(4);
			collisionLineObj->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			collisionLineObj->SetName(obj->GetName() + " collider");
			world->AddGameObject(collisionLineObj);
		}
	}
}