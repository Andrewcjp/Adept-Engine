#include "PhxRigidBody.h"
#if	0


PhxRigidBody::PhxRigidBody()
{

}


PhxRigidBody::~PhxRigidBody()
{

}


PhxRigidBody::PhxRigidBody(physx::PxRigidDynamic* Rigid)
{
	actor = Rigid;
	//if (actor != nullptr) {
	//	const physx::PxU32 shapenumber = actor->getNbShapes();//get the number of shapes
	//	physx::PxShape* shapes[1];//might have to change this
	//	actor->getShapes(shapes, 1);//copy data to the shapes array
	//								//should only be one shape in this object/actor thingimajig!
	//								//otherwise the lastone will be used!
	//	if (shapes[0] != nullptr) {
	//		transform = physx::PxShapeExt::getGlobalPose(*shapes[0], *actor);
	//	}
	//}
}


glm::vec3 PhxRigidBody::GetPosition()
{
#if	PHYSX_ENABLED

	return PXvec3ToGLM(actor->getGlobalPose().p);
#else
	return glm::vec3();
#endif
}

glm::quat PhxRigidBody::GetRotation()
{
#if	PHYSX_ENABLED
	return PXquatToGLM(actor->getGlobalPose().q);
#else
	return glm::quat();
#endif
}

void PhxRigidBody::AddTorque(glm::vec3 torque)
{
	if (actor != nullptr) {
		actor->addTorque(GLMtoPXvec3(torque));
	}
}

void PhxRigidBody::AddForce(glm::vec3 force)
{
	if (actor != nullptr) {
		actor->addForce(GLMtoPXvec3(force));
	}
}

glm::vec3 PhxRigidBody::GetLinearVelocity()
{
	if (actor != nullptr) {
		return PXvec3ToGLM(actor->getLinearVelocity());
	}
	return glm::vec3();
}

#endif