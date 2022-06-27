#pragma once

#include "../CSC8503Common/GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class ColourBlock : public GameObject {
		public:
			ColourBlock();
			~ColourBlock() {};

			void Update(float dt) override;

			void OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) override;

			bool IsColoured() const { return coloured; }

			void StartFade(Vector4 targetColour);

		protected:

			bool coloured;

			Vector4 targetColour;
			Vector4 prevColour;
			float fadeTimer;
			float fadeDuration;
		};
	}
}