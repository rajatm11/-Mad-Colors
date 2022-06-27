#include "Player.h"
#include "Opponent.h"
#include "LevelManager.h"

#include "../CSC8503Common/GameWorld.h"
#include "../../Common/Window.h"

NCL::CSC8503::Player::Player(int agentID, Camera* cam, Vector3 offset, GameWorld* gameWorld, LevelManager* l, vector<ColourBlock*>& wall, const Vector3& startPoint) : Agent(agentID, l, startPoint, wall) {
	name = "Player";

	camera = cam;
	world = gameWorld;

	speed = 20.0f;
	jumpForce = 350.0f;
	damping = 0.98f;

	grounded = false;
	jumpCooldownDuration = 0.15f;
	jumpCooldownTimer = 0.15f;

	paintShotRate = 0.3f;
	paintShotTimer = 0.3f;
	paintShotForce = 10;

	thirdPerson = false;
	controlling = true;
	cameraActive = true;

	respawnDuration = 3.0f;
	respawnTimer = 0;
	respawnFlag = true;

	fpCamOffset = offset;
	tpCamOffset = Vector3(0, 5, 10);
	camOffset = fpCamOffset;
	yaw = 0;

	SetCameraAttached(true);
	transform.SetOrientation(Quaternion::LookRotation(l->GetEnvironmentCentre() - transform.GetPosition(), Vector3(0, 1, 0)));
}

void NCL::CSC8503::Player::Update(float dt) {
	jumpCooldownTimer += dt;
	paintShotTimer += dt;
	respawnTimer += dt;

	if (transform.GetPosition().y < -50 && !isRespawning) {
		isRespawning = true;
		respawnTimer = 0;
		SetControlling(false);
	}

	if (isRespawning && respawnTimer > respawnDuration) {
		isRespawning = false;
		Respawn();
	}

	if (health <= 0 && !isRespawning) {
		isRespawning = true;
		SetControlling(false);
		respawnTimer = 0;
		GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	}

	if (controlling) {
		UpdateMouse();
		UpdateKeys();
	}

	if (cameraAttached) {
		camera->SetPosition(transform.GetPosition() + camOffset);
	}
	if (cameraAttached && cameraActive) {
		camera->SetYaw(yaw);
	}

	if (cameraAttached && controlling) {
		camera->UpdateCamera(dt);
	}

	UpdateGun(dt);

	// Avoid rotating player
	GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
	transform.SetOrientation(Quaternion::EulerAnglesToQuaternion(0, transform.GetOrientation().ToEuler().y, 0));

	// Dampen the player movement to feel more responsive
	GetPhysicsObject()->SetLinearVelocity(GetPhysicsObject()->GetLinearVelocity() * Vector3(damping, 1, damping));
}

void NCL::CSC8503::Player::OnCollisionBegin(GameObject* otherObject, CollisionDetection::ContactPoint point) {
	if (otherObject->GetLayer() == CollisionLayer::FLOOR && jumpCooldownTimer > jumpCooldownDuration) {
		grounded = true;
	}

	if (otherObject->GetName() == "Refill Point") {
		paintAmmo = 12;
	}

	if (otherObject->GetName() == "Projectile") {
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("hurt.wav"), SoundPriority::SOUNDPRIORITY_HIGH, (((float)rand() / RAND_MAX) * 0.6f) + 0.7f);
		health -= 20.0f;
		if (health <= 0) {
			health = 0;
			isRespawning = true;
			SetControlling(false);
			respawnTimer = 0;
			GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
			SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("death.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
			Explode();
		}
	}
}

void NCL::CSC8503::Player::UpdateMouse() {

	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	transform.SetOrientation(Quaternion::EulerAnglesToQuaternion(transform.GetOrientation().x, yaw, transform.GetOrientation().z));

	firingInfo = -1;
	if (Window::GetMouse()->ButtonDown(MouseButtons::RIGHT) && paintShotTimer > paintShotRate && !Debug::IsActive()) {
		if (paintAmmo > 0) {
			ShootPaint(false);
			SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintshot.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
			paintAmmo--;
		}
		else {
			SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("emptygun.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
		}
		paintShotTimer = 0;
	}

	if (Window::GetMouse()->ButtonDown(MouseButtons::LEFT) && paintShotTimer > paintShotRate && !Debug::IsActive()) {
		if (paintAmmo > 0) {
			ShootPaint(true);
			SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("paintshot.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
			paintAmmo--;
		}
		else {
			SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("emptygun.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
		}
		paintShotTimer = 0;
	}
}

void NCL::CSC8503::Player::UpdateKeys() {

	// WASD
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		GetPhysicsObject()->AddForce(transform.GetOrientation() * Vector3(0, 0, -1) * speed);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		GetPhysicsObject()->AddForce(transform.GetOrientation() * Vector3(0, 0, 1) * speed);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		GetPhysicsObject()->AddForce(transform.GetOrientation() * Vector3(-1, 0, 0) * speed);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		GetPhysicsObject()->AddForce(transform.GetOrientation() * Vector3(1, 0, 0) * speed);
	}

	// Jump
	if (grounded && Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
		GetPhysicsObject()->SetLinearVelocity(GetPhysicsObject()->GetLinearVelocity() * Vector3(1, 0, 1));
		GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * jumpForce);
		grounded = false;
		jumpCooldownTimer = 0;
	}
}

void NCL::CSC8503::Player::UpdateGun(float dt) {
	gunColourTimer += dt;
	if (gunColourTimer >= gunColourDuration) {
		prevGunColour = paintGun->GetRenderObject()->GetColour();
		targetGunColour = Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);
		gunColourTimer = 0;
	}
	Quaternion rot = Quaternion::EulerAnglesToQuaternion(camera->GetPitch() + 5, camera->GetYaw() + 5, 0);
	paintGun->GetTransform().SetPosition(camera->GetPosition() + rot * Vector3(0.75, -1.5f, -2));
	paintGun->GetTransform().SetOrientation(rot);
	paintGun->GetRenderObject()->SetColour(Vector4::Lerp(prevGunColour, targetGunColour, gunColourTimer / gunColourDuration));
}

GameObject* NCL::CSC8503::Player::ShootRay() {
	Quaternion camRot = Quaternion::EulerAnglesToQuaternion(camera->GetPitch(), camera->GetYaw(), 0);
	Ray ray = Ray(camera->GetPosition() + camRot * Vector3(0, 0, -2.5f), camRot * Vector3(0, 0, -1));
	GameObject* closestObject;

	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true)) {
		closestObject = (GameObject*)closestCollision.node;
		return closestObject;
	}
	return nullptr;
}

void NCL::CSC8503::Player::ShootPaint(bool coloured) {
	if (networkObject) {
		firingInfo = coloured ? 0 : 1;
	}
	else {
		Quaternion camRot = Quaternion::EulerAnglesToQuaternion(camera->GetPitch(), camera->GetYaw(), 0);
		GameObject* projectile = (GameObject*)level->AddProjectile(camera->GetPosition() + camRot * Vector3(0, 0, -4), coloured);
		projectile->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(camera->GetPitch() + 90, camera->GetYaw(), 90));
		projectile->GetRenderObject()->SetColour(coloured ? Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1) : Vector4(1, 1, 1, 1));
		projectile->GetPhysicsObject()->ApplyLinearImpulse(camRot * Vector3(0, 0, -1) * paintShotForce);
	}
}

void NCL::CSC8503::Player::Respawn() {
	health = 100;
	transform.SetPosition(spawnPos);
	GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	paintAmmo = 0;
	controlling = true;
	SetControlling(true);
}
