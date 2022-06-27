#include "NetworkProjectile.h"
#include "NetworkedGame.h"

void NCL::CSC8503::NetworkProjectile::Update(float dt) {
	timeAlive += dt;
	if (timeAlive >= lifetime) {
		game->OnProjectileDestroyed(networkID, transform.GetPosition());
		Remove();
	}
}

void NCL::CSC8503::NetworkProjectile::OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) {
	int hitPlayerID = -1;
	if (otherObject->GetName() == "Player" || otherObject->GetName() == "Opponent" || otherObject->GetName() == "Agent") {
		hitPlayerID = ((Agent*)otherObject)->GetID();
	}
	else if (otherObject->GetName() == "Colour Block") {
		((ColourBlock*)otherObject)->StartFade(renderObject->GetColour());
	}

	bool paintSplat = otherObject->GetName() != "Player" && otherObject->GetName() != "Opponent" && otherObject->GetName() != "Agent" && otherObject->GetName() != "Refill Point";
	if (paintSplat) {
		level->AddPaintSplat(transform.GetPosition(), point.penetration, point.normal)->GetRenderObject()->SetColour(renderObject->GetColour());
	}

	SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintsplat.wav"), transform.GetPosition(), 150);
	game->OnProjectileDestroyed(networkID, transform.GetPosition(), point.normal, point.penetration, paintSplat, hitPlayerID);
	Remove();
}
