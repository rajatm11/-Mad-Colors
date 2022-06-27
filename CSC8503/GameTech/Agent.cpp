#include "Agent.h"
#include "LevelManager.h"

void NCL::CSC8503::Agent::Update(float dt) {
	UpdateGun(dt);
	if (health <= 0 && respawnFlag) {
		respawnFlag = false;
		isRespawning = true;
		Explode();
	}

	if (isRespawning) {
		respawnTimer += dt;
		if (respawnTimer >= respawnDuration) {
			health = 100;
			isRespawning = false;
			respawnTimer = 0;
			respawnFlag = true;
		}
	}
}

int NCL::CSC8503::Agent::GetNumBlocksColoured() const {
	int numBlocksColoured = 0;
	for (ColourBlock* b : targetWall) {
		numBlocksColoured += b->IsColoured() ? 1 : 0;
	}
	return numBlocksColoured;
}

int NCL::CSC8503::Agent::GetNumBlocksRemaining() const {
	int blocksRemaining = 0;
	for (ColourBlock* block : targetWall) {
		if (!block->IsColoured()) {
			blocksRemaining++;
		}
	}
	return blocksRemaining;
}

void NCL::CSC8503::Agent::Explode() {
	level->AddPaintExplosion(transform.GetPosition() + Vector3(0, 3.5f, 0), 1.5);
}

void NCL::CSC8503::Agent::UpdateGun(float dt) {
	gunColourTimer += dt;
	if (gunColourTimer >= gunColourDuration) {
		prevGunColour = paintGun->GetRenderObject()->GetColour();
		targetGunColour = Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);
		gunColourTimer = 0;
	}
	paintGun->GetTransform().SetPosition(transform.GetPosition() + Vector3(0, 1.5f, 0) + transform.GetOrientation() * Vector3(0.5, 0, -0.5));
	paintGun->GetTransform().SetOrientation(transform.GetOrientation());
	paintGun->GetRenderObject()->SetColour(Vector4::Lerp(prevGunColour, targetGunColour, gunColourTimer / gunColourDuration));
}
