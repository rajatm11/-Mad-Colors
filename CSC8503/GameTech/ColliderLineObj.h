#pragma once
#include "../CSC8503Common/GameObject.h"
#include "LevelManager.h"

#ifndef COLLIDERLINEOBJ_H
#define COLLIDERLINEOBJ_H

namespace NCL {
	namespace CSC8503 {
		class ColliderLine:public GameObject
		{
		public:
			ColliderLine(GameObject* boundingObj,LevelManager* level);
			~ColliderLine() {};

			void Update(float dt) override;
		protected:
			GameObject* boundingObj = nullptr;
		};

		
	}
}

#endif // !COLLIDERLINEOBJ_H
