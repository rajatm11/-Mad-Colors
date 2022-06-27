#include "LevelManager.h"
#include "RefillPoint.h"
#include "Projectile.h"
#include "NetworkProjectile.h"
#include "ColourBlock.h"
#include "NetworkColourBlock.h"
#include "NetworkRefillPoint.h"
#include "PaintSplat.h"

#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/CollisionDetection.h"
#include "../CSC8503Common/NavigationGrid.h"

#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../../Common/Assets.h"

#include <fstream>
#include <OGLMesh.cpp>

const Vector4 COLOUR_RED = Vector4(1, 0, 0, 1);
const Vector4 COLOUR_GREEN = Vector4(0, 1, 0, 1);
const Vector4 COLOUR_BLUE = Vector4(0, 0, 1, 1);

const Vector4 COLOUR_BLACK = Vector4(0, 0, 0, 1);
const Vector4 COLOUR_DARK_GREY = Vector4(0.15f, 0.15f, 0.15f, 1);
const Vector4 COLOUR_GREY = Vector4(0.5f, 0.5f, 0.5f, 1);
const Vector4 COLOUR_LIGHT_GREY = Vector4(0.75f, 0.75f, 0.75f, 1);
const Vector4 COLOUR_WHITE = Vector4(1, 1, 1, 1);

const Vector4 COLOUR_YELLOW = Vector4(1, 1, 0, 1);
const Vector4 COLOUR_MAGENTA = Vector4(1, 0, 1, 1);
const Vector4 COLOUR_CYAN = Vector4(0, 1, 1, 1);

NCL::CSC8503::LevelManager::LevelManager(Game* g, GameWorld& gw) : game(g), world(gw) {
	//InitialiseAssets();
}

NCL::CSC8503::LevelManager::~LevelManager() {

	for (map<string, OGLMesh*>::iterator it = meshMap.begin(); it != meshMap.end(); it++) {
		delete it->second;
	}
	meshMap.clear();

	for (map<string, OGLTexture*>::iterator it = texMap.begin(); it != texMap.end(); it++) {
		delete it->second;
	}
	texMap.clear();

	for (map<string, MeshAnimation*>::iterator it = animMap.begin(); it != animMap.end(); it++) {
		delete it->second;
	}
	animMap.clear();

	for (map<string, MeshMaterial*>::iterator it = materialMap.begin(); it != materialMap.end(); it++) {
		delete it->second;
	}
	materialMap.clear();

	for (map<string, OGLShader*>::iterator it = shaderMap.begin(); it != shaderMap.end(); it++) {
		delete it->second;
	}
	shaderMap.clear();
}

void NCL::CSC8503::LevelManager::InitialiseAssets() {
	assetsLoading = true;

	// Meshes
	assetInfo.push_back(AssetLoadInfo('m', "cube", "cube.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "sphere", "sphere.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "Male1", "Male1.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "courier", "courier.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "security", "security.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "coin", "coin.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "capsule", "capsule.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "Male_Guard", "Male_Guard.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "WoodenBox", "WoodenBox.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "OHW_Ranged_PistolLethal", "OHW_Ranged_PistolLethal.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "THW_Ranged_SMGSoldier", "THW_Ranged_SMGSoldier.msh"));
	assetInfo.push_back(AssetLoadInfo('m', "corridor_Wall_Straight_Mid_end_R", "corridor_Wall_Straight_Mid_end_R.msh"));

	// Textures
	assetInfo.push_back(AssetLoadInfo('t', "WhiteTex", "WhiteTex.png"));
	assetInfo.push_back(AssetLoadInfo('t', "yellowTex", "yellowTex.png"));
	assetInfo.push_back(AssetLoadInfo('t', "stoneTex", "stoneTex.png"));
	assetInfo.push_back(AssetLoadInfo('t', "playerTex", "playerTex.png"));
	assetInfo.push_back(AssetLoadInfo('t', "colourTex", "colourTex.png"));
	assetInfo.push_back(AssetLoadInfo('t', "gun", "gun.png"));
	assetInfo.push_back(AssetLoadInfo('t', "corridor_wall_c", "corridor_wall_c.tga"));
	assetInfo.push_back(AssetLoadInfo('t', "p1", "p1.png"));
	assetInfo.push_back(AssetLoadInfo('t', "p2", "p2.png"));
	assetInfo.push_back(AssetLoadInfo('t', "p3", "p3.png"));
	assetInfo.push_back(AssetLoadInfo('t', "p4", "p4.png"));
	assetInfo.push_back(AssetLoadInfo('t', "default", "checkerboard.png"));
	for (int i = 0; i < 35; ++i) {
		assetInfo.push_back(AssetLoadInfo('t', "splat" + to_string(i), "splat" + (i < 10 ? "0" + to_string(i) : to_string(i)) + ".png"));
	}

	// Materials
	assetInfo.push_back(AssetLoadInfo('e', "wall", "corridor_Wall_Straight_Mid_end_R.mat"));
	assetInfo.push_back(AssetLoadInfo('e', "Male_Guard", "Male_Guard.mat"));

	// Animations
	assetInfo.push_back(AssetLoadInfo('a', "StepForward", "StepForwardRifle.anm"));

	// Shaders
	assetInfo.push_back(AssetLoadInfo('s', "default", "GameTechVert.glsl", "GameTechFrag.glsl"));
	assetInfo.push_back(AssetLoadInfo('s', "box", "GameTechVert.glsl", "BoxFrag.glsl"));
	assetInfo.push_back(AssetLoadInfo('s', "guard", "guardVertex.glsl", "GameTechFrag.glsl"));
	assetInfo.push_back(AssetLoadInfo('s', "line", "lineVertex.glsl", "lineFrag.glsl", "lineGeometry.glsl"));
	// Audio
	assetInfo.push_back(AssetLoadInfo('l', "lol", "lol.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "gunshot", "gunshot.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "paintshot", "paintshot.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "paintsplat", "paintsplat.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "powerup", "powerup.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "hurt", "hurt.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "death", "death.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "emptygun", "emptygun.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "menumusic", "menumusic.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "menuclick", "menuclick.wav"));
	assetInfo.push_back(AssetLoadInfo('l', "end", "end.wav"));
}

float NCL::CSC8503::LevelManager::LoadNextAsset() {
	AssetLoadInfo info = assetInfo[numAssetsLoaded];
	switch (info.type) {
	case 'm':
		LoadMesh(info.identifier, info.filenameOne);
		break;
	case 't':
		LoadTexture(info.identifier, info.filenameOne);
		break;
	case 'e':
		LoadMaterial(info.identifier, info.filenameOne);
		break;
	case 'a':
		LoadAnimation(info.identifier, info.filenameOne);
		break;
	case 's':
		LoadShader(info.identifier, info.filenameOne, info.filenameTwo,info.filenameThree);
		break;
	case 'l':
		LoadSound(info.identifier, info.filenameOne);
		break;
	}

	numAssetsLoaded++;
	float percentComplete = (float)numAssetsLoaded / (float)assetInfo.size() * 100;

	if ((int)percentComplete == 100) {
		InitMaterials();
		assetsLoading = false;
		game->ChangeState(Game::State::MAIN_MENU);
	}
	return percentComplete;
}

GameObject* NCL::CSC8503::LevelManager::AddFloorToWorld(const Vector3& position, const Vector3& dimensions) {
	GameObject* floor = new GameObject("Floor");
	floor->SetLayer(CollisionLayer::FLOOR);

	AABBVolume* volume = new AABBVolume(dimensions);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(dimensions * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), meshMap["cube"], texMap["stoneTex"], shaderMap["default"]));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(floor);

	return floor;
}

GameObject* NCL::CSC8503::LevelManager::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("Sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), meshMap["sphere"], texMap["default"], shaderMap["default"]));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(sphere);

	return sphere;
}

GameObject* NCL::CSC8503::LevelManager::AddAABBCubeToWorld(const Vector3& position, const Vector3& dimensions, float inverseMass) {
	GameObject* cube = new GameObject("AABB Cube");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), meshMap["cube"], texMap["default"], shaderMap["default"]));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);

	return cube;
}

GameObject* NCL::CSC8503::LevelManager::AddOBBCubeToWorld(const Vector3& position, const Vector3& dimensions, float inverseMass) {
	GameObject* cube = new GameObject("OBB Cube");

	OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), meshMap["cube"], texMap["default"], shaderMap["default"]));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);

	return cube;
}

GameObject* NCL::CSC8503::LevelManager::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject("Capsule");

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), meshMap["capsule"], texMap["default"], shaderMap["default"]));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(capsule);

	return capsule;
}

GameObject* NCL::CSC8503::LevelManager::AddCharacterToWorld(const Vector3& position, bool useMeshA) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject("Character");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), useMeshA ? meshMap["Male1"] : meshMap["courier"], nullptr, shaderMap["default"]));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(character);

	return character;
}

GameObject* NCL::CSC8503::LevelManager::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject("Enemy");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), meshMap["security"], nullptr, shaderMap["default"]));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(character);

	return character;
}

GameObject* NCL::CSC8503::LevelManager::AddBonusToWorld(const Vector3& position) {
	GameObject* bonus = new GameObject("Bonus");

	SphereVolume* volume = new SphereVolume(0.25f);
	bonus->SetBoundingVolume((CollisionVolume*)volume);
	bonus->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	bonus->SetRenderObject(new RenderObject(&bonus->GetTransform(), meshMap["coin"], nullptr, shaderMap["default"]));
	bonus->SetPhysicsObject(new PhysicsObject(&bonus->GetTransform(), bonus->GetBoundingVolume()));

	bonus->GetPhysicsObject()->SetInverseMass(1.0f);
	bonus->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(bonus);

	return bonus;
}

SoundEmitter* NCL::CSC8503::LevelManager::AddExampleSoundToWorld(const Vector3& position) {
	Vector3 dimensions(1, 1, 1);

	SoundEmitter* e = new SoundEmitter(Sound::GetSound("lol.wav"));
	e->SetRadius(200.0f);
	e->SetPriority(SoundPriority::SOUNDPRIORITY_ALWAYS);

	e->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	e->SetRenderObject(new RenderObject(&e->GetTransform(), meshMap["cube"], texMap["default"], shaderMap["default"]));
	e->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	SoundSystem::GetSoundSystem()->AddSoundEmitter(e);

	world.AddGameObject(e);

	return e;
}

SoundEmitter* NCL::CSC8503::LevelManager::AddSoundEmitterToWorld(const Vector3& position, const string& soundName, SoundPriority priority, float volume, bool global, bool looping, float radius) {
	SoundEmitter* e = new SoundEmitter(Sound::GetSound(soundName));
	e->SetRadius(radius);
	e->SetIsGlobal(global);
	e->SetLooping(looping);
	e->SetPriority(priority);
	e->SetVolume(volume);

	e->GetTransform().SetPosition(position);

	SoundSystem::GetSoundSystem()->AddSoundEmitter(e);

	world.AddGameObject(e);

	return e;
}

RefillPoint* NCL::CSC8503::LevelManager::AddRefillPoint(const Vector3& position, float radius) {
	RefillPoint* r = new RefillPoint();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	r->SetBoundingVolume((CollisionVolume*)volume);
	r->SetTrigger(true);

	r->GetTransform()
		.SetScale(sphereSize * 3)
		.SetPosition(position);

	r->SetRenderObject(new RenderObject(&r->GetTransform(), meshMap["THW_Ranged_SMGSoldier"], texMap["gun"], shaderMap["default"]));
	r->GetRenderObject()->SetColour(COLOUR_GREEN);

	r->SetPhysicsObject(new PhysicsObject(&r->GetTransform(), r->GetBoundingVolume()));
	r->GetPhysicsObject()->SetInverseMass(0);
	r->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(r);

	return r;
}

NetworkRefillPoint* NCL::CSC8503::LevelManager::AddRefillPoint(int networkID, NetworkedGame* game, bool serverSide, const Vector3& position, float radius) {
	NetworkRefillPoint* r = new NetworkRefillPoint(networkID, game, serverSide);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	r->SetBoundingVolume((CollisionVolume*)volume);
	r->SetTrigger(true);

	r->GetTransform()
		.SetScale(sphereSize * 3)
		.SetPosition(position);

	r->SetRenderObject(new RenderObject(&r->GetTransform(), meshMap["THW_Ranged_SMGSoldier"], texMap["gun"], shaderMap["default"]));
	r->GetRenderObject()->SetColour(COLOUR_GREEN);
	r->SetNetworkObject(new NetworkObject(*r, networkID));
	game->AddNetworkObject(r->GetNetworkObject(), networkID);

	r->SetPhysicsObject(new PhysicsObject(&r->GetTransform(), r->GetBoundingVolume()));
	r->GetPhysicsObject()->SetInverseMass(0);
	r->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(r);

	return r;
}

Projectile* NCL::CSC8503::LevelManager::AddProjectile(const Vector3& position, bool colourProjectile) {
	float radius = 0.25f;
	float inverseMass = 10;

	Projectile* projectile = new Projectile(&world, this, colourProjectile);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	projectile->SetBoundingVolume((CollisionVolume*)volume);

	projectile->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	projectile->SetRenderObject(new RenderObject(&projectile->GetTransform(), meshMap["sphere"], nullptr, shaderMap["default"]));
	projectile->SetPhysicsObject(new PhysicsObject(&projectile->GetTransform(), projectile->GetBoundingVolume()));

	projectile->GetPhysicsObject()->SetInverseMass(inverseMass);
	projectile->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(projectile);

	return projectile;
}

NetworkProjectile* NCL::CSC8503::LevelManager::AddProjectile(int networkID, NetworkedGame* game, const Vector3& position, bool colourProjectile) {
	float radius = 0.25f;
	float inverseMass = 10;

	NetworkProjectile* projectile = new NetworkProjectile(networkID, game, this, &world, colourProjectile);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	projectile->SetBoundingVolume((CollisionVolume*)volume);

	projectile->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	projectile->SetRenderObject(new RenderObject(&projectile->GetTransform(), meshMap["sphere"], nullptr, shaderMap["default"]));
	projectile->SetPhysicsObject(new PhysicsObject(&projectile->GetTransform(), projectile->GetBoundingVolume()));
	projectile->SetNetworkObject(new NetworkObject(*projectile, networkID));

	projectile->GetPhysicsObject()->SetInverseMass(inverseMass);
	projectile->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(projectile);

	return projectile;
}

ColourBlock* NCL::CSC8503::LevelManager::AddColourBlock(const Vector3& position, const Vector3& dimensions) {
	ColourBlock* cBlock = new ColourBlock();

	AABBVolume* volume = new AABBVolume(dimensions);

	cBlock->SetBoundingVolume((CollisionVolume*)volume);

	cBlock->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cBlock->SetRenderObject(new RenderObject(&cBlock->GetTransform(), meshMap["cube"], texMap["WhiteTex"], shaderMap["default"]));
	cBlock->SetPhysicsObject(new PhysicsObject(&cBlock->GetTransform(), cBlock->GetBoundingVolume()));

	cBlock->GetPhysicsObject()->SetInverseMass(0);
	cBlock->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cBlock);

	return cBlock;
}

NetworkColourBlock* NCL::CSC8503::LevelManager::AddColourBlock(int networkID, NetworkedGame* game, const Vector3& position, const Vector3& dimensions) {
	NetworkColourBlock* cBlock = new NetworkColourBlock(networkID, game);

	AABBVolume* volume = new AABBVolume(dimensions);

	cBlock->SetBoundingVolume((CollisionVolume*)volume);

	cBlock->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cBlock->SetRenderObject(new RenderObject(&cBlock->GetTransform(), meshMap["cube"], texMap["WhiteTex"], shaderMap["default"]));
	cBlock->SetPhysicsObject(new PhysicsObject(&cBlock->GetTransform(), cBlock->GetBoundingVolume()));
	cBlock->SetNetworkObject(new NetworkObject(*cBlock, networkID));

	cBlock->GetPhysicsObject()->SetInverseMass(0);
	cBlock->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cBlock);

	return cBlock;
}

GameObject* NCL::CSC8503::LevelManager::AddObstacleBox(const Vector3& position, const Vector3& dimensions, float inverseMass) {
	GameObject* cube = new GameObject("Obstacle Box");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.45);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), meshMap["WoodenBox"], texMap["yellowTex"], shaderMap["box"]));

	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);

	return cube;
}

GameObject* NCL::CSC8503::LevelManager::AddEdgeWall(const Vector3& position, const Vector3& dimensions, const Vector3& direction, const Quaternion& orientation, float inverseMass) {
	GameObject* cube = new GameObject("Edge Wall");

	AABBVolume* volume = new AABBVolume(dimensions * Vector3(1, 3, 1) * 2); //Vector3(-direction.x * dimensions.x, dimensions.y * 3, -direction.z * dimensions.z));

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position + dimensions * direction)
		.SetScale(dimensions * 2)
		.SetOrientation(orientation);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), meshMap["corridor_Wall_Straight_Mid_end_R"], texMap["corridor_wall_c"], shaderMap["default"]));
	cube->GetRenderObject()->SetFlag(3);

	cube->GetRenderObject()->SetTextures(WallTextures);

	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);

	return cube;
}

GameObject* NCL::CSC8503::LevelManager::AddPaintSplat(const Vector3& position, float penetrationDist, const Vector3& normal) {

	PaintSplat* splat = new PaintSplat();

	AABBVolume* volume = new AABBVolume(Vector3(0.005f, 0.005f, 0.005f));

	splat->SetBoundingVolume((CollisionVolume*)volume);

	splat->GetTransform()
		.SetPosition(position + normal * 0.25f)
		.SetScale(Vector3(0.005f, 0.005f, 0.005f));

	string splatShape = to_string(rand() % 35);
	splat->SetRenderObject(new RenderObject(&splat->GetTransform(), meshMap["cube"], texMap["splat" + splatShape], shaderMap["default"]));
	splat->GetRenderObject()->SetRenderShadow(false);

	splat->SetPhysicsObject(new PhysicsObject(&splat->GetTransform(), splat->GetBoundingVolume()));

	if (penetrationDist < 0.1f) {
		splat->GetPhysicsObject()->SetUseGravity(true);
	}
	else {
		splat->GetPhysicsObject()->SetUseGravity(false);
	}

	world.AddGameObject(splat);

	return splat;
}

GameObject* NCL::CSC8503::LevelManager::AddPlayerWallIndicator(int playerID, const Vector3& position, const Vector3& dimensions) {
	GameObject* PInd = new GameObject();

	PInd->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * Vector3(0, 1, 1))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(180, playerID == 1 || playerID == 4 ? -45 : 45, 0));
	
	PInd->SetRenderObject(new RenderObject(&PInd->GetTransform(), meshMap["cube"], texMap["p" + to_string(playerID)], shaderMap["default"]));
	PInd->GetRenderObject()->SetRenderShadow(false);

	world.AddGameObject(PInd);

	return PInd;
}

void NCL::CSC8503::LevelManager::AddPaintExplosion(const Vector3& position, float explosionForce) {
	Vector3 directions[8] = {
		Vector3(1, 0.5, 0),
		Vector3(-1, 0.5, 0),
		Vector3(0, 0.5, 1),
		Vector3(0, 0.5, -1),
		Vector3(1, 0.5, 1),
		Vector3(-1, 0.5, 1),
		Vector3(1, 0.5, -1),
		Vector3(-1, 0.5, -1)
	};

	for (int i = 0; i < 8; ++i) {
		GameObject* projectile = AddProjectile(position + directions[i].Normalised() * 2, true);
		projectile->GetRenderObject()->SetColour(Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1));
		projectile->GetPhysicsObject()->ApplyLinearImpulse(directions[i].Normalised() * explosionForce);
	}
}

void NCL::CSC8503::LevelManager::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void NCL::CSC8503::LevelManager::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddSphereToWorld(position, sphereRadius);
				//AddCubeToWorld(position, cubeDims);
			}
			else {
				AddCapsuleToWorld(position, 2.0f, sphereRadius);
				//AddCubeToWorld(position, cubeDims);
			}
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void NCL::CSC8503::LevelManager::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims, bool useOBB) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			if (useOBB) {
				AddOBBCubeToWorld(position, cubeDims, 1.0f);
			}
			else {
				AddAABBCubeToWorld(position, cubeDims, 1.0f);
			}
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void NCL::CSC8503::LevelManager::InitGameExamples() {
	AddCharacterToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void NCL::CSC8503::LevelManager::LoadEnvironment(const string& levelFile, vector<ColourBlock*> colourWalls[]) {
	int gridHeight;
	int gridWidth;
	int size;
	std::ifstream infile(Assets::DATADIR + levelFile);

	infile >> size;
	infile >> gridWidth;
	infile >> gridHeight;

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			char type = 0;
			infile >> type;
			switch (type) {
			case '1':
				AddPlayerWallIndicator(1, Vector3(x + 0.5f, 5, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case '2':
				AddPlayerWallIndicator(2, Vector3(x + 0.5f, 5, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case '3':
				AddPlayerWallIndicator(3, Vector3(x + 0.5f, 5, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case '4':
				AddPlayerWallIndicator(4, Vector3(x + 0.5f, 5, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case 'i':
				AddObstacleBox(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, 0);
				break;
			case 'x':
				CreateWallOfHeight(3, Vector3(x, 0, y), size, colourWalls[0], COLOUR_WHITE);
				break;
			case 'y':
				CreateWallOfHeight(3, Vector3(x, 0, y), size, colourWalls[1], COLOUR_WHITE);
				break;
			case 'z':
				CreateWallOfHeight(3, Vector3(x, 0, y), size, colourWalls[2], COLOUR_WHITE);
				break;
			case 'w':
				CreateWallOfHeight(3, Vector3(x, 0, y), size, colourWalls[3], COLOUR_WHITE);
				break;
			case 'a':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(0, -1, -1), Quaternion(0, 0, 0, 0), 0);
				break;
			case 'b':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(-1, -1, 0), Quaternion::EulerAnglesToQuaternion(0, 90, 0), 0);
				break;
			case 'c':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(1, -1, 0), Quaternion::EulerAnglesToQuaternion(0, -90, 0), 0);
				break;
			case 'd':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(0, -1, 1), Quaternion::EulerAnglesToQuaternion(0, 180, 0), 0);
				break;
			}
		}
	}
	AddFloorToWorld(Vector3((gridWidth / 2.0f), -0.6f, (gridHeight / 2.0f)) * size, Vector3(gridWidth / 2.0f, 0.1f, gridHeight / 2.0f) * size);

	environmentExtents = Vector2(gridWidth, gridHeight);
	environmentUnitSize = size;
	environmentActive = true;
}

void NCL::CSC8503::LevelManager::LoadEnvironment(NetworkedGame* game, const string& levelFile, vector<ColourBlock*> colourWalls[]) {
	int gridHeight;
	int gridWidth;
	int size;
	std::ifstream infile(Assets::DATADIR + levelFile);

	infile >> size;
	infile >> gridWidth;
	infile >> gridHeight;

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			char type = 0;
			infile >> type;
			switch (type) {
			case '1':
				AddPlayerWallIndicator(1, Vector3(x + 0.5f, 4, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case '2':
				AddPlayerWallIndicator(2, Vector3(x + 0.5f, 4, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case '3':
				AddPlayerWallIndicator(3, Vector3(x + 0.5f, 4, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case '4':
				AddPlayerWallIndicator(4, Vector3(x + 0.5f, 4, y + 0.5f) * size, Vector3(size, size, size) * 2);
				break;
			case 'i':
				AddObstacleBox(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, 0);
				break;
			case 'x':
				CreateWallOfHeight(game, 3, Vector3(x, 0, y), size, colourWalls[0], COLOUR_WHITE);
				break;
			case 'y':
				CreateWallOfHeight(game, 3, Vector3(x, 0, y), size, colourWalls[1], COLOUR_WHITE);
				break;
			case 'z':
				CreateWallOfHeight(game, 3, Vector3(x, 0, y), size, colourWalls[2], COLOUR_WHITE);
				break;
			case 'w':
				CreateWallOfHeight(game, 3, Vector3(x, 0, y), size, colourWalls[3], COLOUR_WHITE);
				break;
			case 'a':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(0, -1, -1), Quaternion(0, 0, 0, 0), 0);
				break;
			case 'b':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(-1, -1, 0), Quaternion::EulerAnglesToQuaternion(0, 90, 0), 0);
				break;
			case 'c':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(1, -1, 0), Quaternion::EulerAnglesToQuaternion(0, -90, 0), 0);
				break;
			case 'd':
				AddEdgeWall(Vector3(x + 0.5f, 0, y + 0.5f) * size, Vector3(size, size, size) * 0.5f, Vector3(0, -1, 1), Quaternion::EulerAnglesToQuaternion(0, 180, 0), 0);
				break;
			}
		}
	}
	AddFloorToWorld(Vector3((gridWidth / 2.0f), -0.6f, (gridHeight / 2.0f)) * size, Vector3(gridWidth / 2.0f, 0.1f, gridHeight / 2.0f) * size);

	environmentExtents = Vector2(gridWidth, gridHeight);
	environmentUnitSize = size;
	environmentActive = true;
}

void NCL::CSC8503::LevelManager::CreateWallOfHeight(int height, const Vector3& position, float blockSize, vector<ColourBlock*> &colourWall, Vector4 wallColour) {
	for (int i = 0; i < height; ++i) {
		ColourBlock* wall = AddColourBlock(Vector3(position.x + 0.5f, i, position.z + 0.5f) * blockSize, Vector3(blockSize, blockSize, blockSize) * 0.5f);
		wall->GetRenderObject()->SetColour(wallColour);
		colourWall.push_back(wall);
	}
}

void NCL::CSC8503::LevelManager::CreateWallOfHeight(NetworkedGame* game, int height, const Vector3& position, float blockSize, vector<ColourBlock*>& colourWall, Vector4 wallColour) {
	for (int i = 0; i < height; ++i) {
		NetworkColourBlock* wall = AddColourBlock(game->GetNextObjectIDAndIncrement(), game, Vector3(position.x + 0.5f, i, position.z + 0.5f) * blockSize, Vector3(blockSize, blockSize, blockSize) * 0.5f);
		game->AddNetworkObject(wall->GetNetworkObject(), wall->GetNetworkID());
		wall->GetRenderObject()->SetColour(wallColour);
		colourWall.push_back(wall);
	}
}

Vector3& NCL::CSC8503::LevelManager::GetEnvironmentCentre() const {
	return Vector3(0, 0, 0) + (environmentActive ? Vector3(environmentExtents.x * 0.5f * environmentUnitSize, 0, environmentExtents.y * 0.5f * environmentUnitSize) : Vector3(0, 0, 0));
}

void NCL::CSC8503::LevelManager::LoadMesh(const string& identifier, const string& filename) {
	OGLMesh* mesh = new OGLMesh(filename);
	(mesh)->SetPrimitiveType(GeometryPrimitive::Triangles);
	(mesh)->UploadToGPU();
	meshMap[identifier] = mesh;
}

void NCL::CSC8503::LevelManager::LoadTexture(const string& identifier, const string& filename) {
	OGLTexture* tex = (OGLTexture*)TextureLoader::LoadAPITexture(filename);
	texMap[identifier] = tex;
}

void NCL::CSC8503::LevelManager::LoadMaterial(const string& identifier, const string& filename) {
	materialMap[identifier] = new MeshMaterial(filename);
}

void NCL::CSC8503::LevelManager::LoadAnimation(const string& identifier, const string& filename) {
	animMap[identifier] = new MeshAnimation(filename);
}

void NCL::CSC8503::LevelManager::LoadShader(const string& identifier, const string& vertName, const string& fragName,const string&geoName) {
	shaderMap[identifier] = new OGLShader(vertName, fragName,geoName);
}

void NCL::CSC8503::LevelManager::LoadSound(const string& identifier, const string& filename) {
	Sound::AddSound(filename);
}

void NCL::CSC8503::LevelManager::InitMaterials() {
	for (int i = 0; i < meshMap["corridor_Wall_Straight_Mid_end_R"]->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = materialMap["wall"]->GetMaterialForLayer(i);

		const std::string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);

		OGLTexture* tmpTex = (OGLTexture*)TextureLoader::LoadAPITexture(*filename);
		WallTextures.emplace_back(tmpTex->GetObjectID());
	}

	for (int i = 0; i < meshMap["Male_Guard"]->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = materialMap["Male_Guard"]->GetMaterialForLayer(i);

		const std::string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);

		OGLTexture* tmpTex = (OGLTexture*)TextureLoader::LoadAPITexture(*filename);
		GuardTextures.emplace_back(tmpTex->GetObjectID());
	}
}
