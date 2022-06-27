#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkProjectile.h"
#include "NetworkColourBlock.h"
#include "NetworkRefillPoint.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame() {
	thisServer = nullptr;
	thisClient = nullptr;
	online = true;
	playerPacketID = 0;

	NetworkBase::Initialise();
}

NetworkedGame::~NetworkedGame() {
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() {
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);

	thisServer->RegisterPacketHandler(Received_State, this);
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(Player_ID, this);
	thisClient->RegisterPacketHandler(Client_Start, this);
	thisClient->RegisterPacketHandler(Disconnect_Confirmation, this);
	thisClient->RegisterPacketHandler(New_Projectile, this);
	thisClient->RegisterPacketHandler(Destroy_Projectile, this);
	thisClient->RegisterPacketHandler(ColourBlockUpdate, this);
	thisClient->RegisterPacketHandler(RefillPointUpdate, this);
	thisClient->RegisterPacketHandler(Game_Timer, this);
}

void NetworkedGame::UpdateGame(float dt) {

	if (thisServer) {
		UpdateAsServer(dt);
	}
	else if (thisClient) {
		UpdateAsClient(dt);
	}

	Game::UpdateGame(dt);
}

void NCL::CSC8503::NetworkedGame::ChangeState(State newState) {

	SoundSystem::GetSoundSystem()->SetMasterVolume(1);

	if (newState == State::MAIN_MENU) {
		Reset();
		world->ClearAndErase();
		physics->Clear();
		world->GetMainCamera()->SetYaw(105.0f);
		world->GetMainCamera()->SetPitch(5.0f);
		InitListener();
		gameTimer = 90;
		gameOver = false;
		gameUI->ResetPlayers();
		gameUI->SetValid(true);
		Window::GetWindow()->ShowOSPointer(true);
		Debug::Reset();
		levelManager->AddSoundEmitterToWorld(Vector3(1000, 1000, 1000), "menumusic.wav", SoundPriority::SOUNDPRIORITY_HIGH, 0.15f, true, true);
	}

	if (online && newState == State::WAITING) {
		InitWorld();
	}
	else if (activeState == State::MAIN_MENU && newState == State::PLAYING) {
		Game::InitWorld();
	}

	if (activeState == State::WAITING && newState == State::PLAYING) {
		InitCamera();
		gameOver = false;
	}

	if (newState == State::PAUSED) {
		if (!gameOver) {
			SoundSystem::GetSoundSystem()->SetMasterVolume(0);
		}
		if (localPlayer) localPlayer->SetCameraAttached(false, false);
		if (!online) player->SetCameraAttached(false, false);
		Window::GetWindow()->ShowOSPointer(true);
	}

	if (newState == State::PLAYING) {
		if (localPlayer) localPlayer->SetCameraAttached(true, false);
		if (!online) player->SetCameraAttached(true, false);
		Window::GetWindow()->ShowOSPointer(false);
	}

	activeState = newState;
}

void NCL::CSC8503::NetworkedGame::UpdateWaitingState(float dt) {
	world->GetMainCamera()->SetYaw(world->GetMainCamera()->GetYaw() + 0.025f);
	HandleUICommand();
}

void NetworkedGame::UpdateAsServer(float dt) {
	thisServer->UpdateServer();
	BroadcastSnapshot(false);
}

void NetworkedGame::UpdateAsClient(float dt) {

	thisClient->UpdateClient();

	if (localPlayer) {
		ClientPacket newPacket;
		newPacket.playerID = localPlayer->GetID();
		newPacket.prevPacketID = playerPacketID;
		playerPacketID++;

		Vector3 playerPos = localPlayer->GetTransform().GetPosition();
		newPacket.position[0] = playerPos.x;
		newPacket.position[1] = playerPos.y;
		newPacket.position[2] = playerPos.z;

		Quaternion playerOri = localPlayer->GetTransform().GetOrientation();
		newPacket.orientation[0] = playerOri.x;
		newPacket.orientation[1] = playerOri.y;
		newPacket.orientation[2] = playerOri.z;
		newPacket.orientation[3] = playerOri.w;

		newPacket.pitch = localPlayer->GetPitch();
		newPacket.yaw = localPlayer->GetYaw();
		newPacket.firingInfo = localPlayer->GetFiringInfo();

		thisClient->SendPacket(newPacket);
	}
}

bool NCL::CSC8503::NetworkedGame::ConnectClient(string& fullIP) {
	string delimiter = ".";
	size_t pos = 0;
	string ipSections[4];
	int index = 0;
	while ((pos = fullIP.find(delimiter)) != string::npos) {
		ipSections[index] = fullIP.substr(0, pos);
		fullIP.erase(0, pos + delimiter.length());
		index++;
	}
	ipSections[index] = fullIP.substr(0, fullIP.length());
	try {
		StartAsClient(stoi(ipSections[0]), stoi(ipSections[1]), stoi(ipSections[2]), stoi(ipSections[3]));
		return true;
	}
	catch (...) {
		string msg = "Couldn't connect";
		Debug::PrintToConsole(msg);
		return false;
	}
}

void NCL::CSC8503::NetworkedGame::ConnectPlayer() {
	if (nextPlayerID < 4 && !serverPlayers[nextPlayerID]) {
		serverPlayers[nextPlayerID] = AddAgentToWorld(nextPlayerID, nextObjectID, spawnPoints[nextPlayerID], colourWallMap[nextPlayerID]);
		networkObjects[nextObjectID] = serverPlayers[nextPlayerID]->GetNetworkObject();
		gameUI->SetPlayer(nextPlayerID, serverPlayers[nextPlayerID]);
		thisServer->SendPacketToPeer(nextPlayerID, PlayerIDPacket(nextPlayerID, nextObjectID));
		nextPlayerID++;
		nextObjectID++;
	}
}

void NCL::CSC8503::NetworkedGame::UpdatePlayer(ClientPacket* packet) {
	Agent* player = serverPlayers[packet->playerID];
	player->GetTransform().SetPosition(Vector3(packet->position[0], packet->position[1], packet->position[2]));
	player->GetTransform().SetOrientation(Quaternion(packet->orientation[0], packet->orientation[1], packet->orientation[2], packet->orientation[3]));
	if (packet->firingInfo == 0 || packet->firingInfo == 1) {
		FireProjectile(packet);
	}
}

void NCL::CSC8503::NetworkedGame::InitialiseLocalPlayer(int playerID) {
	localPlayer = AddPlayerToWorld(playerID, -1, spawnPoints[playerID], colourWallMap[playerID]);
	serverPlayers[playerID] = localPlayer;
	thisClient->SendPacket(ClientPacket());
	gameUI->SetPlayer(0, localPlayer);
	localPlayer->SetCameraAttached(false, false);
}

void NCL::CSC8503::NetworkedGame::InitialiseNetworkPlayer(int playerID, int objectID) {
	serverPlayers[playerID] = AddAgentToWorld(playerID, objectID, spawnPoints[playerID], colourWallMap[playerID]);
	networkObjects[objectID] = serverPlayers[playerID]->GetNetworkObject();
	gameUI->SetPlayer(gameUI->GetNumPlayers(), serverPlayers[playerID]);
}

void NCL::CSC8503::NetworkedGame::UpdateNetworkPlayer(FullPacket* packet) {
	Agent* player = serverPlayers[packet->playerID];
	player->GetTransform().SetPosition(packet->fullState.position);
	player->GetTransform().SetOrientation(packet->fullState.orientation);
}

void NCL::CSC8503::NetworkedGame::UpdateObjectState(FullPacket* packet) {
	if (packet->playerID != -1 && packet->playerID != localPlayer->GetID()) {
		if (!serverPlayers[packet->playerID]) {
			InitialiseNetworkPlayer(packet->playerID, packet->objectID);
		}
		else {
			UpdateNetworkPlayer(packet);
		}
	}
	else if (packet->objectID != -1 && packet->playerID != localPlayer->GetID() && networkObjects[packet->objectID]) {
		networkObjects[packet->objectID]->ReadPacket(*packet);
	}
}

void NCL::CSC8503::NetworkedGame::HandleUICommand() {
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
			online = false;
			ChangeState(State::PLAYING);
			break;
		case GameUI::Command::HOST_SERVER:
			online = true;
			if (!thisServer) {
				StartAsServer();
				ChangeState(State::WAITING);
			}
			break;
		case GameUI::Command::CONNECT_TO_SERVER:
			online = true;
			if (thisClient) {
				ChangeState(State::WAITING);
			}
			break;
		case GameUI::Command::EXIT_GAME:
			isPlaying = false;
			break;
		}
	} else if (activeState == State::WAITING && command == GameUI::Command::START_MULTIPLAYER) {
		ChangeState(State::PLAYING);
		thisServer->SendGlobalPacket(ClientStartPacket());
	}
}

void NCL::CSC8503::NetworkedGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitListener();

	vector<ColourBlock*> colourWalls[4];

	levelManager->LoadEnvironment(this, "LevelData.txt", colourWalls);
	mapGrid = new NavigationGrid("LevelLayout.txt");
	refillPoints.push_back(levelManager->AddRefillPoint(nextObjectID++, this, thisServer, levelManager->GetEnvironmentCentre() - Vector3(0, 2, 0), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(nextObjectID++, this, thisServer, levelManager->GetEnvironmentCentre() + Vector3(50, -2, 0), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(nextObjectID++, this, thisServer, levelManager->GetEnvironmentCentre() + Vector3(0, -2, 50), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(nextObjectID++, this, thisServer, levelManager->GetEnvironmentCentre() - Vector3(50, 2, 0), 2.5f));
	refillPoints.push_back(levelManager->AddRefillPoint(nextObjectID++, this, thisServer, levelManager->GetEnvironmentCentre() - Vector3(0, 2, 50), 2.5f));

	spawnPoints[0] = Vector3(40, 0, 40);
	spawnPoints[1] = Vector3(160, 0, 40);
	spawnPoints[2] = Vector3(40, 0, 160);
	spawnPoints[3] = Vector3(160, 0, 160);

	colourWallMap[0] = colourWalls[0];
	colourWallMap[1] = colourWalls[1];
	colourWallMap[2] = colourWalls[2];
	colourWallMap[3] = colourWalls[3];

	world->BuildStaticTree();
}

void NCL::CSC8503::NetworkedGame::DetermineWinners() {
	if (!online) {
		Game::DetermineWinners();
		return;
	}
	int bestScore = 0;
	for (int i = 0; i < 4; ++i) {
		int numBlocksColoured = serverPlayers[i] ? serverPlayers[i]->GetNumBlocksColoured() : 0;
		if (numBlocksColoured > bestScore) {
			bestScore = numBlocksColoured;
			winners.clear();
			winners.push_back(serverPlayers[i]);
		}
		else if (numBlocksColoured == bestScore) {
			if (serverPlayers[i]) winners.push_back(serverPlayers[i]);
		}
	}
}

void NCL::CSC8503::NetworkedGame::Reset() {
	if (thisClient) {
		thisClient->Disconnect();
		delete thisClient;
		thisClient = nullptr;
		if (localPlayer) localPlayer->Remove();
		localPlayer = nullptr;
	}
	else if (thisServer) {
		delete thisServer;
		thisServer = nullptr;
	}
	networkObjects.clear();
	serverPlayers.clear();
	nextObjectID = 0;
	nextPlayerID = 0;
	playerPacketID = 0;
}

Player* NCL::CSC8503::NetworkedGame::AddPlayerToWorld(int agentID, int objectID, const Vector3& position, vector<ColourBlock*>& wall) {
	float halfHeight = 2;
	float radius = 1;
	float inverseMass = 5;

	Player* player = new Player(agentID, world->GetMainCamera(), Vector3(0, 3.5f, 0), world, levelManager, wall, position);

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius, Vector3(0, 2, 0));
	player->SetBoundingVolume((CollisionVolume*)volume);

	player->GetTransform()
		.SetScale(Vector3(radius * -2, halfHeight, radius * 1.5))
		.SetPosition(position);

	player->SetRenderObject(new RenderObject(&player->GetTransform(), levelManager->GetMesh("capsule"), levelManager->GetDefaultTexture(), levelManager->GetDefaultShader()));
	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume()));
	player->SetNetworkObject(new NetworkObject(*player, objectID));

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

Agent* NCL::CSC8503::NetworkedGame::AddAgentToWorld(int agentID, int objectID, const Vector3& position, vector<ColourBlock*>& wall) {
	float halfHeight = 2;
	float radius = 1;
	float inverseMass = 5;

	Agent* agent = new Agent(agentID, levelManager, position, wall);

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius, Vector3(0, 2, 0));
	agent->SetBoundingVolume((CollisionVolume*)volume);

	agent->GetTransform()
		.SetScale(Vector3(radius * -2, halfHeight, radius * 2))
		.SetPosition(position);

	agent->SetRenderObject(new RenderObject(&agent->GetTransform(), levelManager->GetMesh("Male_Guard"), levelManager->GetDefaultTexture(), levelManager->GetShader("guard")));
	agent->SetPhysicsObject(new PhysicsObject(&agent->GetTransform(), agent->GetBoundingVolume()));
	agent->SetNetworkObject(new NetworkObject(*agent, objectID));

	agent->GetRenderObject()->SetFlag(1);
	agent->GetRenderObject()->SetTextures(levelManager->GuardTextures);
	agent->GetRenderObject()->SetAnimation(levelManager->GetAnimation("StepForward"));

	agent->GetPhysicsObject()->SetInverseMass(inverseMass);
	agent->GetPhysicsObject()->InitCubeInertia();
	agent->GetPhysicsObject()->SetElasticity(0.1f);

	GameObject* gun = new GameObject();
	gun->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2) * Vector3(2, 2, 2))
		.SetPosition(position);

	gun->SetRenderObject(new RenderObject(&gun->GetTransform(), levelManager->GetMesh("THW_Ranged_SMGSoldier"), levelManager->GetTexture("gun"), levelManager->GetDefaultShader()));
	agent->SetGunObj(gun);

	world->AddGameObject(agent);
	world->AddGameObject(gun);

	return agent;
}

void NCL::CSC8503::NetworkedGame::FireProjectile(ClientPacket* packet) {
	Quaternion camRot = Quaternion::EulerAnglesToQuaternion(packet->pitch, packet->yaw, 0);
	Vector3 camPos = Vector3(packet->position[0], packet->position[1], packet->position[2]) + Vector3(0, 3.5f, 0);
	GameObject* projectile = (GameObject*) levelManager->AddProjectile(nextObjectID, this, camPos + camRot * Vector3(0, 0, -4), packet->firingInfo == 0);
	projectile->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(packet->pitch + 90, packet->yaw, 90));
	projectile->GetRenderObject()->SetColour(packet->firingInfo == 0 ? Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1) : Vector4(1, 1, 1, 1));
	projectile->GetPhysicsObject()->ApplyLinearImpulse(camRot * Vector3(0, 0, -1) * paintShotForce);
	networkObjects[nextObjectID] = projectile->GetNetworkObject();

	ProjectilePacket newPacket;
	newPacket.objectID = nextObjectID;
	nextObjectID++;

	Vector3 projectilePos = projectile->GetTransform().GetPosition();
	newPacket.position[0] = projectilePos.x;
	newPacket.position[1] = projectilePos.y;
	newPacket.position[2] = projectilePos.z;

	Vector4 colour = projectile->GetRenderObject()->GetColour();
	newPacket.colour[0] = colour.x;
	newPacket.colour[1] = colour.y;
	newPacket.colour[2] = colour.z;
	newPacket.colour[3] = colour.w;

	thisServer->SendGlobalPacket(newPacket);
}

void NCL::CSC8503::NetworkedGame::FireProjectile(ProjectilePacket* packet) {
	GameObject* projectile = levelManager->AddSphereToWorld(Vector3(packet->position[0], packet->position[1], packet->position[2]) , 0.25f, 10);
	projectile->SetNetworkObject(new NetworkObject(*projectile, packet->objectID));
	projectile->GetRenderObject()->SetDefaultTexture(nullptr);
	projectile->GetRenderObject()->SetColour(Vector4(packet->colour[0], packet->colour[1], packet->colour[2], packet->colour[3]));
	networkObjects[packet->objectID] = projectile->GetNetworkObject();
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}

		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, 0)) {
			thisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}
	}

	thisServer->SendGlobalPacket(GameTimerPacket(gameTimer));
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	if (type == Received_State) {
		ClientPacket* realPacket = (ClientPacket*)payload;
		if (!serverPlayers[realPacket->playerID]) {
			ConnectPlayer();
		}
		else {
			UpdatePlayer(realPacket);
		}
	}

	if (type == Player_ID) {
		PlayerIDPacket* realPacket = (PlayerIDPacket*)payload;
		if (!localPlayer) {
			InitialiseLocalPlayer(realPacket->playerID);
		}
		else if (localPlayer->GetID() == realPacket->playerID) {
			localPlayer->GetNetworkObject()->SetNetworkID(realPacket->objectID);
			networkObjects[realPacket->objectID] = localPlayer->GetNetworkObject();
		}
	}

	if (type == Client_Start) {
		ChangeState(State::PLAYING);
	}

	if (type == Disconnect_Confirmation) {
		DisconnectConfirmPacket* realPacket = (DisconnectConfirmPacket*)payload;
		string msg = std::to_string(realPacket->playerID);
		Debug::PrintToConsole(msg);

		if (realPacket->playerID == localPlayer->GetID()) {
			delete thisClient;
			thisClient = nullptr;
		}
	}

	if (type == Full_State) {
		FullPacket* realPacket = (FullPacket*)payload;
		UpdateObjectState(realPacket);
	}

	if (type == New_Projectile) {
		ProjectilePacket* realPacket = (ProjectilePacket*)payload;
		FireProjectile(realPacket);
	}

	if (type == Destroy_Projectile) {
		DestroyProjectilePacket* realPacket = (DestroyProjectilePacket*)payload;
		OnProjectileDestroyed(realPacket->objectID, 
			Vector3(realPacket->position[0], realPacket->position[1], realPacket->position[2]), 
			Vector3(realPacket->normal[0], realPacket->normal[1], realPacket->normal[2]),
			realPacket->penetration,
			realPacket->paintSplat,
			realPacket->hitPlayerID
		);
	}

	if (type == ColourBlockUpdate) {
		ColourBlockPacket* realPacket = (ColourBlockPacket*)payload;
		OnWallBlockColoured(realPacket->objectID, Vector4(realPacket->colour[0], realPacket->colour[1], realPacket->colour[2], realPacket->colour[3]), realPacket->coloured);
	}

	if (type == RefillPointUpdate) {
		RefillPointPacket* realPacket = (RefillPointPacket*)payload;
		OnRefillPointStateChanged(realPacket->objectID, realPacket->available, realPacket->collectPlayerID);
	}

	if (type == Game_Timer) {
		GameTimerPacket* realPacket = (GameTimerPacket*)payload;
		gameTimer = realPacket->gameTime;
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);
	}
}

void NCL::CSC8503::NetworkedGame::OnProjectileDestroyed(int objectID, Vector3 position, Vector3 normal, float penetration, bool paintSplat, int playerID) {
	if (thisServer) {
		if (playerID != -1) serverPlayers[playerID]->SetHealth(serverPlayers[playerID]->GetHealth() - 20);
		DestroyProjectilePacket newPacket;
		newPacket.objectID = objectID;
		newPacket.position[0] = position.x;
		newPacket.position[1] = position.y;
		newPacket.position[2] = position.z;
		newPacket.normal[0] = normal.x;
		newPacket.normal[1] = normal.y;
		newPacket.normal[2] = normal.z;
		newPacket.penetration = penetration;
		newPacket.paintSplat = paintSplat;
		newPacket.hitPlayerID = playerID;
		thisServer->SendGlobalPacket(newPacket);
	}
	else if (thisClient) {
		Vector4 col = networkObjects[objectID]->GetGameObject()->GetRenderObject()->GetColour();
		networkObjects[objectID]->GetGameObject()->Remove();
		if (paintSplat) {
			levelManager->AddPaintSplat(position, penetration, normal)->GetRenderObject()->SetColour(col);
		}
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintsplat.wav"), position, 150);
		if (playerID != -1) {
			Agent* a = serverPlayers[playerID];
			a->SetHealth(a->GetHealth() - 20);
			if (a->GetHealth() == 0) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), a->GetTransform().GetPosition(), 150);
			}
			else if (a->GetHealth() > 0) {
				SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("hurt.wav"), a->GetTransform().GetPosition(), 150, (((float)rand() / RAND_MAX) * 0.6f) + 0.7f);
			}
		}
	}
}

void NCL::CSC8503::NetworkedGame::OnWallBlockColoured(int objectID, Vector4 colour, bool coloured) {
	if (thisServer) {
		ColourBlockPacket newPacket;
		newPacket.objectID = objectID;
		newPacket.coloured = coloured;
		
		newPacket.colour[0] = colour.x;
		newPacket.colour[1] = colour.y;
		newPacket.colour[2] = colour.z;
		newPacket.colour[3] = colour.w;

		thisServer->SendGlobalPacket(newPacket);
	}
	else if (thisClient) {
		NetworkColourBlock* block = static_cast<NetworkColourBlock*>(networkObjects[objectID]->GetGameObject());
		block->SetColoured(coloured);
		block->StartFade(colour);
	}
}

void NCL::CSC8503::NetworkedGame::OnRefillPointStateChanged(int objectID, bool available, int playerID) {
	if (thisServer) {
		RefillPointPacket newPacket;
		newPacket.objectID = objectID;
		newPacket.available = available;
		newPacket.collectPlayerID = playerID;

		thisServer->SendGlobalPacket(newPacket);
	}
	else if (thisClient) {
		NetworkRefillPoint* refillPoint = static_cast<NetworkRefillPoint*>(networkObjects[objectID]->GetGameObject());
		refillPoint->SetActive(available);
		if (!available) {
			SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("powerup.wav"), refillPoint->GetTransform().GetPosition(), 100);
		}
		if (playerID != -1) {
			serverPlayers[playerID]->SetAmmo(12);
		}
	}
}

int NCL::CSC8503::NetworkedGame::GetNextObjectIDAndIncrement() {
	return nextObjectID++;
}

void NCL::CSC8503::NetworkedGame::AddNetworkObject(NetworkObject* o, int networkID) {
	networkObjects[networkID] = o;
}
