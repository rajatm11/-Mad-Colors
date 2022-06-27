#include "ColliderLineObj.h"

NCL::CSC8503::ColliderLine::ColliderLine(GameObject* bObj, LevelManager* level) {
	boundingObj = bObj;
	this->SetTransform(boundingObj->GetTransform());
	auto meshMap = level->GetMeshMap();
	auto shaderMap = level->GetShaderMap();
	auto texMap = level->GetTexMap();
	
	auto s = boundingObj->GetTransform().GetScale();
	auto p = boundingObj->GetTransform().GetPosition();
	auto m = boundingObj->GetTransform().GetMatrix();
	auto o = boundingObj->GetTransform().GetOrientation();
	this->GetTransform().
		SetScale(s).
		SetPosition(p).
		SetOrientation(o).
		SetMatrix(m);


	switch (boundingObj->GetBoundingVolume()->type)
	{
	case VolumeType::AABB:
		renderObject = new RenderObject(&transform, meshMap["cube"], texMap["default"], shaderMap["line"]);
		break;
	case VolumeType::OBB:
		renderObject = new RenderObject(&transform, meshMap["cube"], texMap["default"], shaderMap["line"]);
		break;
	case VolumeType::Sphere:
		renderObject = new RenderObject(&transform, meshMap["sphere"], texMap["default"], shaderMap["line"]);
		break;
	case VolumeType::Capsule:
		renderObject = new RenderObject(&transform, meshMap["capsule"], texMap["default"], shaderMap["line"]);
		break;
	default:
		break;
	}
	renderObject->SetRenderShadow(false);
	
}


void NCL::CSC8503::ColliderLine::Update(float dt) {
	if (boundingObj != nullptr) {
		auto s = boundingObj->GetTransform().GetScale();
		auto p = boundingObj->GetTransform().GetPosition();
		auto m = boundingObj->GetTransform().GetMatrix();
		auto o = boundingObj->GetTransform().GetOrientation();
		this->GetTransform().
			SetScale(s).
			SetPosition(p).
			SetOrientation(o).
			SetMatrix(m);
		if (boundingObj->GetBoundingVolume()->type == VolumeType::Capsule) {
			auto p = this->GetTransform().GetPosition();
			this->GetTransform().SetPosition(p + Vector3(0, 2, 0));
		}
		if (boundingObj->GetName() == "Obstacle Box") {
			auto s = this->GetTransform().GetScale();
			this->GetTransform().SetScale(s * 0.83);
		}
		if (boundingObj->GetName() == "Colour Block") {
			auto s = this->GetTransform().GetScale();
			this->GetTransform().SetScale(s * 1.01);
		}
		if (boundingObj->GetName() == "Edge Wall") {
			auto s = this->GetTransform().GetScale();
			this->GetTransform().SetScale(s * Vector3(1,3,1)*2);
		}
		if (boundingObj->GetName() == "Refill Point") {
			auto s = this->GetTransform().GetScale();
			this->GetTransform().SetScale(s /3);
		}
	}
	/*else {
		Remove();
	}*/
}