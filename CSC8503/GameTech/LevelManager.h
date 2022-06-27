#pragma once

#include "GameTechRenderer.h"
#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/SoundSystem.h"
#include <map>

#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H
namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class Agent;
		class ColourBlock;
		class NetworkColourBlock;
		class RefillPoint;
		class Projectile;
		class PlayerWallIndicator;
		class NetworkProjectile;
		class NetworkRefillPoint;
		class NetworkedGame;

		class LevelManager {
		public:
			LevelManager(Game* g, GameWorld& gw);
			~LevelManager();

			void InitialiseAssets();
			float LoadNextAsset();

			bool IsLoadingAssets() const { return assetsLoading; }

			OGLMesh*		GetMesh(const string& name)			{ return meshMap[name]; }
			OGLTexture*		GetTexture(const string& name)		{ return texMap[name]; }
			MeshAnimation* GetAnimation(const string& name)		{ return animMap[name]; }
			OGLShader*		GetShader(const string& name)		{ return shaderMap[name]; }
			MeshMaterial*	GetMaterial(const string& name)		{ return materialMap[name]; }

			OGLTexture*	GetDefaultTexture()						{ return texMap["default"]; }
			OGLShader* GetDefaultShader()						{ return shaderMap["default"]; }
			map<string, OGLMesh*> GetMeshMap()					{ return meshMap; }
			map<string, OGLShader*> GetShaderMap()				{ return shaderMap; }
			map<string, OGLTexture*> GetTexMap()				{ return texMap; }

			// Generic
			GameObject* AddFloorToWorld(const Vector3& position, const Vector3& dimensions = Vector3(100, 2, 100));
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddAABBCubeToWorld(const Vector3& position, const Vector3& dimensions, float inverseMass = 10.0f);
			GameObject* AddOBBCubeToWorld(const Vector3& position, const Vector3& dimensions, float inverseMass = 10.0f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddCharacterToWorld(const Vector3& position, bool useMeshA = true);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			SoundEmitter* AddExampleSoundToWorld(const Vector3& position);
			SoundEmitter* AddSoundEmitterToWorld(const Vector3& position, const string& soundName, SoundPriority priority = SoundPriority::SOUNDPRIORITY_MEDIUM, float volume = 1, bool global = false,  bool looping = true, float radius = 500);

			// Game-specific
			RefillPoint* AddRefillPoint(const Vector3& position, float radius);
			NetworkRefillPoint* AddRefillPoint(int networkID, NetworkedGame* game, bool serverSide, const Vector3& position, float radius);
			Projectile* AddProjectile(const Vector3& position, bool colourProjectile = false);
			NetworkProjectile* AddProjectile(int networkID, NetworkedGame* game, const Vector3& position, bool colourProjectile = false);
			ColourBlock* AddColourBlock(const Vector3& position, const Vector3& dimensions);
			NetworkColourBlock* AddColourBlock(int networkID, NetworkedGame* game, const Vector3& position, const Vector3& dimensions);
			GameObject* AddObstacleBox(const Vector3& position, const Vector3& dimensions, float inverseMass);
			GameObject* AddEdgeWall(const Vector3& position, const Vector3& dimensions, const Vector3& renderDir, const Quaternion& orientation, float inverseMass);
			GameObject* AddPaintSplat(const Vector3& position, float penetrationDist, const Vector3& normal);
			GameObject* AddPlayerWallIndicator(int playerID, const Vector3& position, const Vector3& dimensions);
			void AddPaintExplosion(const Vector3& position, float explosionForce = 10);

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims, bool useOBB = true);
			void InitGameExamples();

			void LoadEnvironment(const string& levelFile, vector<ColourBlock*> colourWalls[]);
			void LoadEnvironment(NetworkedGame* game, const string& levelFile, vector<ColourBlock*> colourWalls[]);
			void CreateWallOfHeight(int height, const Vector3& position, float blockSize, vector<ColourBlock*>& colourWall, Vector4 wallColour = Vector4(1, 1, 1, 1));
			void CreateWallOfHeight(NetworkedGame* game, int height, const Vector3& position, float blockSize, vector<ColourBlock*>& colourWall, Vector4 wallColour = Vector4(1, 1, 1, 1));
			Vector3& GetEnvironmentCentre() const;

			//8508
			vector<GLuint> GuardTextures;
			vector<GLuint> WallTextures;


		protected:

			struct AssetLoadInfo {
				AssetLoadInfo(char t, string i, string f1, string f2 = "", string f3 = "") {
					type = t;
					identifier = i;
					filenameOne = f1;
					filenameTwo = f2;
					filenameThree = f3;
				}

				char type;
				string identifier;
				string filenameOne;
				string filenameTwo;
				string filenameThree;
			};


			Game* game;
			GameWorld& world;

			Vector2 environmentExtents = Vector2(1, 1);
			float environmentUnitSize = 1;
			bool environmentActive = false;

			bool assetsLoading = false;
			float progress = 0;
			int numAssetsToLoad = 12;
			int numAssetsLoaded = 0;

			void LoadMesh(const string& identifier, const string& filename);
			void LoadTexture(const string& identifier, const string& filename);
			void LoadMaterial(const string& identifier, const string& filename);
			void LoadAnimation(const string& identifier, const string& filename);
			void LoadShader(const string& identifier, const string& vertName, const string& fragName,const string& geoName);
			void LoadSound(const string& identifier, const string& filename);

			void InitMaterials();

			vector<AssetLoadInfo> assetInfo;

			map<string, OGLMesh*> meshMap;
			map<string, OGLTexture*> texMap;
			map<string, MeshAnimation*> animMap;
			map<string, OGLShader*> shaderMap;
			map<string, MeshMaterial*> materialMap;

		};
	}
}
#endif // !LEVELMANAGER_H