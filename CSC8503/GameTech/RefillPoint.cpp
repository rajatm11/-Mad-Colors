#include "RefillPoint.h"
#include "../CSC8503Common/SoundSystem.h"

NCL::CSC8503::RefillPoint::RefillPoint() {
	name = "Refill Point";

	cooldownDuration = 15.0f;
	cooldownTimer = 0;
	isActive = true;

	rotationSpeed = 0.5f;
}

void NCL::CSC8503::RefillPoint::Update(float dt) {
	cooldownTimer += dt;
	if (cooldownTimer > cooldownDuration && !isActive) {
		Activate();
	}

	transform.SetOrientation(Quaternion::EulerAnglesToQuaternion(0, transform.GetOrientation().ToEuler().y + rotationSpeed, 0));
}

void NCL::CSC8503::RefillPoint::OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) {
	if (otherObject->GetName() == "Player" || otherObject->GetName() == "Opponent") {
		Deactivate();
		cooldownTimer = 0;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("powerup.wav"), transform.GetPosition(), 100);
	}
}

void NCL::CSC8503::RefillPoint::Activate() {
	GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	SetLayer(CollisionLayer::DEFAULT);
	isActive = true;
}

void NCL::CSC8503::RefillPoint::Deactivate() {
	GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	SetLayer(CollisionLayer::IGNORE_DEFAULT);
	isActive = false;
}
