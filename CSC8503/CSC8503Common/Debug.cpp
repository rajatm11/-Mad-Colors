#include "Debug.h"
#include "../../Common/Matrix4.h"

#include <iomanip>
#include <windows.h>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

using namespace NCL;

OGLRenderer* Debug::renderer = nullptr;
Debug* Debug::instance = nullptr;

std::vector<Debug::DebugStringEntry>	Debug::stringEntries;
std::vector<Debug::DebugLineEntry>		Debug::lineEntries;

const Vector4 Debug::RED	= Vector4(1, 0, 0, 1);
const Vector4 Debug::GREEN	= Vector4(0, 1, 0, 1);
const Vector4 Debug::BLUE	= Vector4(0, 0, 1, 1);

const Vector4 Debug::BLACK	= Vector4(0, 0, 0, 1);
const Vector4 Debug::WHITE	= Vector4(1, 1, 1, 1);

const Vector4 Debug::YELLOW		= Vector4(1, 1, 0, 1);
const Vector4 Debug::MAGENTA	= Vector4(1, 0, 1, 1);
const Vector4 Debug::CYAN		= Vector4(0, 1, 1, 1);


void Debug::Print(const std::string& text, const Vector2&pos, const Vector4& colour) {
	DebugStringEntry newEntry;

	newEntry.data		= text;
	newEntry.position	= pos;
	newEntry.colour		= colour;

	stringEntries.emplace_back(newEntry);
}

void Debug::DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour, float time) {
	DebugLineEntry newEntry;

	newEntry.start	= startpoint;
	newEntry.end	= endpoint;
	newEntry.colour = colour;
	newEntry.time	= time;

	lineEntries.emplace_back(newEntry);
}

void Debug::DrawAxisLines(const Matrix4& modelMatrix, float scaleBoost, float time) {
	Matrix4 local = modelMatrix;
	local.SetPositionVector({0, 0, 0});

	Vector3 fwd		= local * Vector4(0, 0, -1, 1.0f);
	Vector3 up		= local * Vector4(0, 1, 0, 1.0f);
	Vector3 right	= local * Vector4(1, 0, 0, 1.0f);

	Vector3 worldPos = modelMatrix.GetPositionVector();

	DrawLine(worldPos, worldPos + (right * scaleBoost)	, Debug::RED, time);
	DrawLine(worldPos, worldPos + (up * scaleBoost)		, Debug::GREEN, time);
	DrawLine(worldPos, worldPos + (fwd * scaleBoost)	, Debug::BLUE, time);
}

void NCL::CSC8503::Debug::DrawCollider(GameObject* g, const Vector4& colour) {
	if (!instance->isActive) {
		return;
	}

	const Transform& worldTransform = g->GetTransform();
	const CollisionVolume* volume = g->GetBoundingVolume();

	if (!volume) {
		return;
	}

	switch (volume->type) {
	case VolumeType::AABB:		DrawAABBCollider(worldTransform, (const AABBVolume&)*volume, colour); break;
	case VolumeType::OBB:		DrawOBBCollider(worldTransform, (const OBBVolume&)*volume, colour); break;
	case VolumeType::Sphere:	DrawSphereCollider(worldTransform, (const SphereVolume&)*volume, colour); break;
	case VolumeType::Capsule:	DrawCapsuleCollider(worldTransform, (const CapsuleVolume&)*volume, colour); break;
	}
}

void NCL::CSC8503::Debug::DrawAABBCollider(const Transform& worldTransform, const AABBVolume& volume, const Vector4& colour) {
	Vector3 center = worldTransform.GetPosition() + volume.GetOffset();
	Vector3 corners[8] = {
		center + volume.GetHalfDimensions() * Vector3(1, 1, 1),
		center + volume.GetHalfDimensions() * Vector3(1, -1, 1),
		center + volume.GetHalfDimensions() * Vector3(1, -1, -1),
		center + volume.GetHalfDimensions() * Vector3(1, 1, -1),
		center + volume.GetHalfDimensions() * Vector3(-1, 1, 1),
		center + volume.GetHalfDimensions() * Vector3(-1, -1, 1),
		center + volume.GetHalfDimensions() * Vector3(-1, -1, -1),
		center + volume.GetHalfDimensions() * Vector3(-1, 1, -1),
	};

	DrawLine(corners[0], corners[1], colour);
	DrawLine(corners[1], corners[2], colour);
	DrawLine(corners[2], corners[3], colour);
	DrawLine(corners[3], corners[0], colour);
	DrawLine(corners[0], corners[4], colour);
	DrawLine(corners[4], corners[7], colour);
	DrawLine(corners[7], corners[3], colour);
	DrawLine(corners[7], corners[3], colour);
	DrawLine(corners[7], corners[6], colour);
	DrawLine(corners[6], corners[2], colour);
	DrawLine(corners[1], corners[5], colour);
	DrawLine(corners[4], corners[5], colour);
	DrawLine(corners[6], corners[5], colour);
}

void NCL::CSC8503::Debug::DrawOBBCollider(const Transform& worldTransform, const OBBVolume& volume, const Vector4& colour) {
	Vector3 center = worldTransform.GetPosition() + volume.GetOffset();
	Vector3 corners[8] = {
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(1, 1, 1)),
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(1, -1, 1)),
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(1, -1, -1)),
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(1, 1, -1)),
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(-1, 1, 1)),
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(-1, -1, 1)),
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(-1, -1, -1)),
		center + volume.GetHalfDimensions() * (worldTransform.GetOrientation() * Vector3(-1, 1, -1)),
	};

	DrawLine(corners[0], corners[1], colour);
	DrawLine(corners[1], corners[2], colour);
	DrawLine(corners[2], corners[3], colour);
	DrawLine(corners[3], corners[0], colour);
	DrawLine(corners[0], corners[4], colour);
	DrawLine(corners[4], corners[7], colour);
	DrawLine(corners[7], corners[3], colour);
	DrawLine(corners[7], corners[3], colour);
	DrawLine(corners[7], corners[6], colour);
	DrawLine(corners[6], corners[2], colour);
	DrawLine(corners[1], corners[5], colour);
	DrawLine(corners[4], corners[5], colour);
	DrawLine(corners[6], corners[5], colour);
}

void NCL::CSC8503::Debug::DrawSphereCollider(const Transform& worldTransform, const SphereVolume& volume, const Vector4& colour) {
	Vector3 center = worldTransform.GetPosition() + volume.GetOffset();

	Vector3 points[30] = {
		center + Vector3(0, 1, 0) * volume.GetRadius(),

		center + Vector3(0.25, 0.75, 0).Normalised() * volume.GetRadius(),
		center + Vector3(0.5, 0.5, 0).Normalised() * volume.GetRadius(),
		center + Vector3(0.75, 0.25, 0).Normalised() * volume.GetRadius(),
		center + Vector3(1, 0, 0) * volume.GetRadius(),

		center + Vector3(-0.25, 0.75, 0).Normalised() * volume.GetRadius(),
		center + Vector3(-0.5, 0.5, 0).Normalised() * volume.GetRadius(),
		center + Vector3(-0.75, 0.25, 0).Normalised() * volume.GetRadius(),
		center + Vector3(-1, 0, 0) * volume.GetRadius(),

		center + Vector3(0, 0.75, 0.25).Normalised() * volume.GetRadius(),
		center + Vector3(0, 0.5, 0.5).Normalised() * volume.GetRadius(),
		center + Vector3(0, 0.25, 0.75).Normalised() * volume.GetRadius(),
		center + Vector3(0, 0, 1) * volume.GetRadius(),

		center + Vector3(0, 0.75, -0.25).Normalised() * volume.GetRadius(),
		center + Vector3(0, 0.5, -0.5).Normalised() * volume.GetRadius(),
		center + Vector3(0, 0.25, -0.75).Normalised() * volume.GetRadius(),
		center + Vector3(0, 0, -1) * volume.GetRadius(),

		center + Vector3(0, -1, 0) * volume.GetRadius(),

		center + Vector3(0.25, -0.75, 0).Normalised() * volume.GetRadius(),
		center + Vector3(0.5, -0.5, 0).Normalised() * volume.GetRadius(),
		center + Vector3(0.75, -0.25, 0).Normalised() * volume.GetRadius(),

		center + Vector3(-0.25, -0.75, 0).Normalised() * volume.GetRadius(),
		center + Vector3(-0.5, -0.5, 0).Normalised() * volume.GetRadius(),
		center + Vector3(-0.75, -0.25, 0).Normalised() * volume.GetRadius(),

		center + Vector3(0, -0.75, 0.25).Normalised() * volume.GetRadius(),
		center + Vector3(0, -0.5, 0.5).Normalised() * volume.GetRadius(),
		center + Vector3(0, -0.25, 0.75).Normalised() * volume.GetRadius(),

		center + Vector3(0, -0.75, -0.25).Normalised() * volume.GetRadius(),
		center + Vector3(0, -0.5, -0.5).Normalised() * volume.GetRadius(),
		center + Vector3(0, -0.25, -0.75).Normalised() * volume.GetRadius(),

	};

	DrawLine(points[0], points[1], colour);
	DrawLine(points[1], points[2], colour);
	DrawLine(points[2], points[3], colour);
	DrawLine(points[3], points[4], colour);

	DrawLine(points[0], points[5], colour);
	DrawLine(points[5], points[6], colour);
	DrawLine(points[6], points[7], colour);
	DrawLine(points[7], points[8], colour);

	DrawLine(points[0], points[9], colour);
	DrawLine(points[9], points[10], colour);
	DrawLine(points[10], points[11], colour);
	DrawLine(points[11], points[12], colour);

	DrawLine(points[0], points[13], colour);
	DrawLine(points[13], points[14], colour);
	DrawLine(points[14], points[15], colour);
	DrawLine(points[15], points[16], colour);

	DrawLine(points[17], points[18], colour);
	DrawLine(points[18], points[19], colour);
	DrawLine(points[19], points[20], colour);
	DrawLine(points[20], points[4], colour);

	DrawLine(points[17], points[21], colour);
	DrawLine(points[21], points[22], colour);
	DrawLine(points[22], points[23], colour);
	DrawLine(points[23], points[8], colour);

	DrawLine(points[17], points[24], colour);
	DrawLine(points[24], points[25], colour);
	DrawLine(points[25], points[26], colour);
	DrawLine(points[26], points[12], colour);

	DrawLine(points[17], points[27], colour);
	DrawLine(points[27], points[28], colour);
	DrawLine(points[28], points[29], colour);
	DrawLine(points[29], points[16], colour);

}

void NCL::CSC8503::Debug::DrawCapsuleCollider(const Transform& worldTransform, const CapsuleVolume& volume, const Vector4& colour) {
	Vector3 center = worldTransform.GetPosition() + volume.GetOffset();
	Vector3 top = center + Vector3(0, volume.GetHalfHeight() - volume.GetRadius(), 0);
	Vector3 bottom = center - Vector3(0, volume.GetHalfHeight() - volume.GetRadius(), 0);

	Vector3 points[30] = {
		top + Vector3(0, 1, 0) * volume.GetRadius(),

		top + Vector3(0.25, 0.75, 0).Normalised() * volume.GetRadius(),
		top + Vector3(0.5, 0.5, 0).Normalised() * volume.GetRadius(),
		top + Vector3(0.75, 0.25, 0).Normalised() * volume.GetRadius(),
		top + Vector3(1, 0, 0) * volume.GetRadius(),

		top + Vector3(-0.25, 0.75, 0).Normalised() * volume.GetRadius(),
		top + Vector3(-0.5, 0.5, 0).Normalised() * volume.GetRadius(),
		top + Vector3(-0.75, 0.25, 0).Normalised() * volume.GetRadius(),
		top + Vector3(-1, 0, 0) * volume.GetRadius(),

		top + Vector3(0, 0.75, 0.25).Normalised() * volume.GetRadius(),
		top + Vector3(0, 0.5, 0.5).Normalised() * volume.GetRadius(),
		top + Vector3(0, 0.25, 0.75).Normalised() * volume.GetRadius(),
		top + Vector3(0, 0, 1) * volume.GetRadius(),

		top + Vector3(0, 0.75, -0.25).Normalised() * volume.GetRadius(),
		top + Vector3(0, 0.5, -0.5).Normalised() * volume.GetRadius(),
		top + Vector3(0, 0.25, -0.75).Normalised() * volume.GetRadius(),
		top + Vector3(0, 0, -1) * volume.GetRadius(),

		bottom + Vector3(0, -1, 0) * volume.GetRadius(),

		bottom + Vector3(0.25, -0.75, 0).Normalised() * volume.GetRadius(),
		bottom + Vector3(0.5, -0.5, 0).Normalised() * volume.GetRadius(),
		bottom + Vector3(0.75, -0.25, 0).Normalised() * volume.GetRadius(),

		bottom + Vector3(-0.25, -0.75, 0).Normalised() * volume.GetRadius(),
		bottom + Vector3(-0.5, -0.5, 0).Normalised() * volume.GetRadius(),
		bottom + Vector3(-0.75, -0.25, 0).Normalised() * volume.GetRadius(),

		bottom + Vector3(0, -0.75, 0.25).Normalised() * volume.GetRadius(),
		bottom + Vector3(0, -0.5, 0.5).Normalised() * volume.GetRadius(),
		bottom + Vector3(0, -0.25, 0.75).Normalised() * volume.GetRadius(),

		bottom + Vector3(0, -0.75, -0.25).Normalised() * volume.GetRadius(),
		bottom + Vector3(0, -0.5, -0.5).Normalised() * volume.GetRadius(),
		bottom + Vector3(0, -0.25, -0.75).Normalised() * volume.GetRadius(),

	};

	DrawLine(points[0], points[1], colour);
	DrawLine(points[1], points[2], colour);
	DrawLine(points[2], points[3], colour);
	DrawLine(points[3], points[4], colour);

	DrawLine(points[0], points[5], colour);
	DrawLine(points[5], points[6], colour);
	DrawLine(points[6], points[7], colour);
	DrawLine(points[7], points[8], colour);

	DrawLine(points[0], points[9], colour);
	DrawLine(points[9], points[10], colour);
	DrawLine(points[10], points[11], colour);
	DrawLine(points[11], points[12], colour);

	DrawLine(points[0], points[13], colour);
	DrawLine(points[13], points[14], colour);
	DrawLine(points[14], points[15], colour);
	DrawLine(points[15], points[16], colour);

	DrawLine(points[17], points[18], colour);
	DrawLine(points[18], points[19], colour);
	DrawLine(points[19], points[20], colour);
	DrawLine(points[20], points[4], colour);

	DrawLine(points[17], points[21], colour);
	DrawLine(points[21], points[22], colour);
	DrawLine(points[22], points[23], colour);
	DrawLine(points[23], points[8], colour);

	DrawLine(points[17], points[24], colour);
	DrawLine(points[24], points[25], colour);
	DrawLine(points[25], points[26], colour);
	DrawLine(points[26], points[12], colour);

	DrawLine(points[17], points[27], colour);
	DrawLine(points[27], points[28], colour);
	DrawLine(points[28], points[29], colour);
	DrawLine(points[29], points[16], colour);

}

void NCL::CSC8503::Debug::UpdateInfo(float dt) {

	SetFPS(CalcFPS(dt));
	SetMemoryUsage(CalcMemory());
}

void Debug::FlushRenderables(float dt) {
	if (!renderer || !instance || !instance->isActive) {
		return;
	}
	for (const auto& i : stringEntries) {
		renderer->DrawString(i.data, i.position);
	}
	if (instance->selectedObject) {
		DrawCollider(instance->selectedObject, Debug::RED);
	}
	if (!instance->showCollisionVolumes) {
		return;
	}
	int trim = 0;
	for (int i = 0; i < lineEntries.size(); ) {
		DebugLineEntry* e = &lineEntries[i]; 
		renderer->DrawLine(e->start, e->end, e->colour);
		e->time -= dt;
		if (e->time < 0) {			
			trim++;				
			lineEntries[i] = lineEntries[lineEntries.size() - trim];
		}
		else {
			++i;
		}		
		if (i + trim >= lineEntries.size()) {
			break;
		}
	}
	lineEntries.resize(lineEntries.size() - trim);

	stringEntries.clear();
}

float NCL::CSC8503::Debug::CalcFPS(float dt)
{
	dtAvg = (dtAvg * fpsSmoothing) + (dt * (1.0f - fpsSmoothing));
	if (fpsTimer > 0.1f) {
		fpsTimer = 0;
		fps = 1.0f / dtAvg;
	}
	fpsTimer += dt;
	return fps;
}

float NCL::CSC8503::Debug::CalcMemory()
{
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	//float memoryUsage_G = pmc.WorkingSetSize / (1024.0 * 1024.0 * 1024.0);
	float memoryUsage_M = pmc.WorkingSetSize / (1024.0 * 1024.0);
	//float memoryUsage_K = pmc.WorkingSetSize / 1024.0;

	return memoryUsage_M;
}
