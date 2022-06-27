#include "Projectile.h"
#include "LevelManager.h"

#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/SoundSystem.h"

NCL::CSC8503::Projectile::Projectile(GameWorld* g, LevelManager* l, bool c) {
	world = g;
	level = l;
	colourProjectile = c;

	name = "Projectile";
	lifetime = 10.0f;
	timeAlive = 0;
	isTrigger = true;
}

void NCL::CSC8503::Projectile::Update(float dt) {
	timeAlive += dt;
	if (timeAlive >= lifetime) {
		Remove();
	}
}

void NCL::CSC8503::Projectile::OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) {
	if (otherObject->GetName() != "Agent" && otherObject->GetName() != "Player" && otherObject->GetName() != "Opponent" && otherObject->GetName() != "Refill Point") {
		level->AddPaintSplat(transform.GetPosition(), point.penetration, point.normal)->GetRenderObject()->SetColour(renderObject->GetColour());
	}
	SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintsplat.wav"), transform.GetPosition(), 150);
	Remove();
}
