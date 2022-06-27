#include "ColourBlock.h"
#include "Projectile.h"

NCL::CSC8503::ColourBlock::ColourBlock() {
	name = "Colour Block";
	coloured = false;
	fadeDuration = 3;
	fadeTimer = 3;
}

void NCL::CSC8503::ColourBlock::Update(float dt) {
	if (fadeTimer < fadeDuration) {
		fadeTimer += dt;
		renderObject->SetColour(Vector4::Lerp(prevColour, targetColour, fadeTimer / fadeDuration));
	}
}

void NCL::CSC8503::ColourBlock::OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) {
	if (otherObject->GetName() == "Projectile") {
		Projectile* p = static_cast<Projectile*>(otherObject);
		coloured = p->IsColoured();
		StartFade(p->GetRenderObject()->GetColour());
	}
}

void NCL::CSC8503::ColourBlock::StartFade(Vector4 targetCol) {
	targetColour = targetCol;
	prevColour = renderObject->GetColour();
	fadeTimer = 0;
}
