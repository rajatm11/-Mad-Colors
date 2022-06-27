#pragma once
#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "GameObject.h"
#include <vector>
#include <string>

namespace NCL {
	namespace CSC8503 {
		class Debug
		{
		public:
			static void Initialise() {
				if (!instance) instance = new Debug();
			}
			static void Destroy() {
				if (instance) {
					delete instance;
					instance = nullptr;
				}
			}

			static Debug* GetInstance() { return instance; }

			static void Reset() {
				if (instance) {
					instance->isActive = false;
					instance->selectedObject = nullptr;
					instance->applyGravity = true;
					instance->freeCam = false;
					instance->aiActive = true;
					instance->showCollisionVolumes = true;
					instance->showCollisionMeshes = false;
				}
			}

			static void PrintToConsole(const string& message) {
				if (instance && instance->isActive) {
					std::cout << message << std::endl;
				}
			}

			static void SetActive(bool a) { if (instance) instance->isActive = a; }
			static bool IsActive() { return instance ? instance->isActive : false; }

			static void Print(const std::string& text, const Vector2& pos, const Vector4& colour = Vector4(1, 1, 1, 1));
			static void DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour = Vector4(1, 1, 1, 1), float time = 0.0f);

			static void SetSelectedObject(GameObject* o) { if (instance) instance->selectedObject = o; }
			static GameObject* GetSelectedObject() { return instance ? instance->selectedObject : nullptr; }

			static void DrawAxisLines(const Matrix4& modelMatrix, float scaleBoost = 1.0f, float time = 0.0f);

			static void DrawCollider(GameObject* g, const Vector4& colour = Vector4(0, 1, 0, 1));
			static void DrawAABBCollider(const Transform& worldTransform, const AABBVolume& volume, const Vector4& colour = Vector4(0, 1, 0, 1));
			static void DrawOBBCollider(const Transform& worldTransform, const OBBVolume& volume, const Vector4& colour = Vector4(0, 1, 0, 1));
			static void DrawSphereCollider(const Transform& worldTransform, const SphereVolume& volume, const Vector4& colour = Vector4(0, 1, 0, 1));
			static void DrawCapsuleCollider(const Transform& worldTransform, const CapsuleVolume& volume, const Vector4& colour = Vector4(0, 1, 0, 1));

			static void SetAIActive(bool a) { if (instance) instance->aiActive = a; }
			static bool GetAIActive() { return instance ? instance->aiActive : true; }

			static void SetApplyGravity(bool g) { if (instance) instance->applyGravity = g; }
			static bool GetApplyGravity() { return instance ? instance->applyGravity : true; }

			static void SetFreeCam(bool f) { if (instance) instance->freeCam = f; }
			static bool GetFreeCam() { return instance ? instance->freeCam : false; }

			static void SetShowCollisionVolumes(bool s) {
				if (instance) {
					instance->showCollisionVolumes = s;
					if (instance->showCollisionVolumes && instance->showCollisionMeshes)
						instance->showCollisionMeshes = false;
				}
			}

			static bool GetShowCollisionVolumes() { return instance ? instance->showCollisionVolumes : true; }

			static void SetShowCollisionMeshes(bool s) {
				if (instance) {
					instance->showCollisionMeshes = s;
					if (instance->showCollisionMeshes && instance->showCollisionVolumes)
						instance->showCollisionVolumes = false;
				}
			}

			static bool GetShowCollisionMeshes() { return instance ? instance->showCollisionMeshes : true; }

			static void SetFullScreen(bool f) { if (instance) instance->fullScreen = f; }
			static bool GetFullScreen() { return instance ? instance->fullScreen : true; }

			static void SetFPS(float val) { if (instance) instance->fps = val; }
			static void SetMemoryUsage(float val) { if (instance) instance->memoryUse = val; }
			static void SetNumTotalObjects(float val) { if (instance) instance->numTotalObjects = val; }
			static void SetNumBroadphaseCollisions(float val) { if (instance) instance->numBroadphaseCollisions = val; }
			static void SetNumNarrowphaseCollisions(float val) { if (instance) instance->numNarrowphaseCollisions = val; }

			static float GetFPS() { return instance ? instance->fps : 0; }
			static float GetMemoryUsage() { return instance ? instance->memoryUse : 0; }
			static int GetNumTotalObjects() { return instance ? instance->numTotalObjects : 0; }
			static int GetNumBroadphaseCollisions() { return instance ? instance->numBroadphaseCollisions : 0; }
			static int GetNumNarrowphaseCollisions() { return instance ? instance->numNarrowphaseCollisions : 0; }

			void UpdateInfo(float dt);

			static void SetRenderer(OGLRenderer* r) {
				renderer = r;
			}

			static void FlushRenderables(float dt);

			static const Vector4 RED;
			static const Vector4 GREEN;
			static const Vector4 BLUE;

			static const Vector4 BLACK;
			static const Vector4 WHITE;

			static const Vector4 YELLOW;
			static const Vector4 MAGENTA;
			static const Vector4 CYAN;

		protected:
			struct DebugStringEntry {
				std::string	data;
				Vector2 position;
				Vector4 colour;
			};

			struct DebugLineEntry {
				Vector3 start;
				Vector3 end;
				float	time;
				Vector4 colour;
			};

			Debug() {}
			~Debug() {}

			float CalcFPS(float dt);
			float CalcMemory();

			static std::vector<DebugStringEntry>	stringEntries;
			static std::vector<DebugLineEntry>		lineEntries;

			static OGLRenderer* renderer;

			static Debug* instance;

			bool isActive = false;

			GameObject* selectedObject;

			bool aiActive = true;
			bool showCollisionVolumes = true;
			bool showCollisionMeshes = false;
			bool applyGravity = true;
			bool freeCam = false;
			bool fullScreen = true;

			float fpsSmoothing = 0.9f;
			float dtAvg = 0;
			float fpsTimer = 0;

			float fps = 0;
			float memoryUse = 0;
			int numTotalObjects = 0;
			int numBroadphaseCollisions = 0;
			int numNarrowphaseCollisions = 0;

		};
	}
}

