#include "NetworkRefillPoint.h"
#include "NetworkedGame.h"

void NCL::CSC8503::NetworkRefillPoint::Update(float dt) {
	cooldownTimer += dt;
	if (serverSide && cooldownTimer > cooldownDuration && !isActive) {
		Activate();
	}

	transform.SetOrientation(Quaternion::EulerAnglesToQuaternion(0, transform.GetOrientation().ToEuler().y + rotationSpeed, 0));
}

void NCL::CSC8503::NetworkRefillPoint::OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) {
	if (serverSide && otherObject->GetName() == "Agent") {
		Deactivate(((Agent*) otherObject)->GetID());
		cooldownTimer = 0;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("powerup.wav"), transform.GetPosition(), 100);
	}
}

void NCL::CSC8503::NetworkRefillPoint::Activate() {
	GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	SetLayer(CollisionLayer::DEFAULT);
	isActive = true;
	if (serverSide) game->OnRefillPointStateChanged(networkID, true);
}

void NCL::CSC8503::NetworkRefillPoint::Deactivate(int collectedPlayerID) {
	GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	SetLayer(CollisionLayer::IGNORE_DEFAULT);
	isActive = false;
	if (serverSide) game->OnRefillPointStateChanged(networkID, false, collectedPlayerID);
}
