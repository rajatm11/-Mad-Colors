#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector3.h"
namespace NCL {
	class OBBVolume : public CollisionVolume
	{
	public:
		OBBVolume(const Maths::Vector3& halfDims, const Vector3& offset = Vector3(0, 0, 0)) : CollisionVolume(offset) {
			type		= VolumeType::OBB;
			halfSizes	= halfDims;
		}
		~OBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}
	protected:
		Maths::Vector3 halfSizes;
	};
}

