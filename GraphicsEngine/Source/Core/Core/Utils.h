#pragma once


#ifndef UTILS_H
#define UTILS_H
glm::vec3 PXvec3ToGLM(physx::PxVec3 val)
{
	return glm::vec3(val.x, val.y, val.z);
}
glm::quat PXquatToGLM(physx::PxQuat val)
{
	return glm::quat(val.x, val.y, val.z, val.w);
}
physx::PxVec3 GLMtoPXvec3(glm::vec3 val)
{
	return physx::PxVec3(val.x, val.y, val.z);
}

#endif /* UTILS_H */