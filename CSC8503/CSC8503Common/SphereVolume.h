#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : public CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f, const Vector3& offset = Vector3(0, 0, 0)) : CollisionVolume(offset) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}
	protected:
		float	radius;
	};
}

