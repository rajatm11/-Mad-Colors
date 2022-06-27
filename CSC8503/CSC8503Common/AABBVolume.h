#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector3.h"
namespace NCL {
	class AABBVolume : public CollisionVolume
	{
	public:
		AABBVolume(const Vector3& halfDims, const Vector3& offset = Vector3(0, 0, 0)) : CollisionVolume(offset) {
			type		= VolumeType::AABB;
			halfSizes	= halfDims;
		}
		~AABBVolume() {

		}

		Vector3 GetHalfDimensions() const {
			return halfSizes;
		}

	protected:
		Vector3 halfSizes;
	};
}
