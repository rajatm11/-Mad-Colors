#pragma once
#include <vector>
#include <string>
#ifndef _MESHANIMATION_H_
#define _MESHANIMATION_H_
namespace NCL {

namespace Maths {
	class Matrix4;
}
using namespace Maths;

class MeshAnimation
{
public:
	MeshAnimation();
	MeshAnimation(const std::string& filename);
	virtual ~MeshAnimation();

	unsigned int GetJointCount() const {
		return jointCount;
	}

	unsigned int GetFrameCount() const {
		return frameCount;
	}

	float GetFrameRate() const {
		return frameRate;
	}

	const Matrix4* GetJointData(unsigned int frame) const;

protected:
	unsigned int	jointCount;
	unsigned int	frameCount;
	float			frameRate;

	std::vector<Matrix4>		allJoints;
};
}
#endif // !_MESHANIMATION_H_
