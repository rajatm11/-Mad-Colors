#pragma once
namespace NCL {
	enum class VolumeType {
		AABB	= 1,
		OBB		= 2,
		Sphere	= 4, 
		Mesh	= 8,
		Capsule = 16,
		Compound= 32,
		Invalid = 256
	};

	class CollisionVolume {
	public:
		CollisionVolume(Vector3 offset = Vector3(0, 0, 0)) {
			type = VolumeType::Invalid;
			this->offset = offset;
		}
		~CollisionVolume() {}

		Vector3 GetOffset() const { return offset; }

		VolumeType type;	

	protected:
		Vector3 offset;
	};
}