#pragma once

#include "Transform.h"
#include "CollisionVolume.h"
#include "CollisionLayer.h"
#include "CollisionDetection.h"

#include "PhysicsObject.h"
#include "RenderObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {
		class NetworkObject;
		class Debug;

		class GameObject {
		public:
			GameObject(string name = "");
			virtual ~GameObject();

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			bool IsSleeping() const {
				return isSleeping;
			}

			void SetSleeping(bool s) {
				isSleeping = s;
			}

			Transform& GetTransform() {
				return transform;
			}

			void SetTransform(Transform t) { transform = t; }

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			NetworkObject* GetNetworkObject() const {
				return networkObject;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			void SetNetworkObject(NetworkObject* newObject) {
				networkObject = newObject;
			}

			void SetName(const string& n) {
				name = n;
			}

			const string& GetName() const {
				return name;
			}

			const CollisionLayer GetLayer() const {
				return layer;
			}

			void SetLayer(CollisionLayer l) {
				layer = l;
			}

			virtual void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point);

			virtual void OnCollisionEnd(GameObject* otherObject);

			bool IsTrigger() const {
				return isTrigger;
			}

			void SetTrigger(bool t) {
				isTrigger = t;
			}

			void Remove() {
				flagForRemoval = true;
			}

			bool ToRemove() const {
				return flagForRemoval;
			}

			virtual void Update(float dt) {};

			bool GetBroadphaseAABB(Vector3& outsize) const;

			void UpdateBroadphaseAABB();

			void SetWorldID(int newID) {
				worldID = newID;
			}

			int	GetWorldID() const {
				return worldID;
			}

			bool IsStatic() const {
				return physicsObject && physicsObject->GetInverseMass() == 0;
			}

			

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;
			NetworkObject*		networkObject;

			bool			flagForRemoval;
			bool			isTrigger;
			bool			isActive;
			bool			isSleeping;
			int				worldID;
			string			name;
			CollisionLayer  layer;

			Vector3 broadphaseAABB;

		};
	}
}
