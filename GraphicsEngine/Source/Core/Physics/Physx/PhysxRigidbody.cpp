
#if PHYSX_ENABLED
#include "PhysxRigidbody.h"
#include "PhysxEngine.h"
#include "Physics/PhysicsTypes.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Engine.h"
using namespace physx;
PhysxRigidbody::~PhysxRigidbody()
{
	//PMaterial->release(); //Not released as we don't own it the shape does
	PhysxEngine::GetPlayScene()->removeActor(*CommonActorPtr);
	CommonActorPtr->release();
	MemoryUtils::DeleteVector(AttachedColliders);
	for (int i = 0; i < Shapes.size(); i++)
	{
		Shapes[i]->userData = (void*)0xAffffffff;
		Shapes[i]->release();
	}
}

PhysxRigidbody::PhysxRigidbody(EBodyType::Type type, Transform initalpos) :GenericRigidBody(type)
{
	transform = initalpos;
}

glm::vec3 PhysxRigidbody::GetPosition() const
{
	return PhysxEngine::PXvec3ToGLM(CommonActorPtr->getGlobalPose().p);
}

glm::quat PhysxRigidbody::GetRotation() const
{
	return PhysxEngine::PXquatToGLM(CommonActorPtr->getGlobalPose().q);
}

void PhysxRigidbody::SetPositionAndRotation(glm::vec3 pos, glm::quat rot)
{
	if (CommonActorPtr != nullptr)
	{
		CommonActorPtr->setGlobalPose(PxTransform(PhysxEngine::GLMtoPXvec3(pos), PhysxEngine::GLMtoPXQuat(rot)));
	}
}

void PhysxRigidbody::AddTorque(glm::vec3 torque)
{
	if (Dynamicactor != nullptr)
	{
		Dynamicactor->addTorque(PhysxEngine::GLMtoPXvec3(torque));
	}
}

void PhysxRigidbody::AddForce(glm::vec3 force, EForceMode::Type Mode)
{
	if (Dynamicactor != nullptr)
	{
		Dynamicactor->addForce(PhysxEngine::GLMtoPXvec3(force));
	}
}

glm::vec3 PhysxRigidbody::GetLinearVelocity() const
{
	if (Dynamicactor != nullptr)
	{
		return PhysxEngine::PXvec3ToGLM(Dynamicactor->getLinearVelocity());
	}
	return glm::vec3();
}

void PhysxRigidbody::SetLinearVelocity(glm::vec3 velocity)
{
	if (Dynamicactor != nullptr)
	{
		Dynamicactor->setLinearVelocity(PhysxEngine::GLMtoPXvec3(velocity));
	}
}
glm::vec3 PhysxRigidbody::GetAngularVelocity() const
{
	if (Dynamicactor != nullptr)
	{
		return PhysxEngine::PXvec3ToGLM(Dynamicactor->getAngularVelocity());
	}
	return glm::vec3();
}
void PhysxRigidbody::SetAngularVelocity(glm::vec3 velocity)
{
	if (Dynamicactor != nullptr)
	{
		Dynamicactor->setAngularVelocity(PhysxEngine::GLMtoPXvec3(velocity));
	}
}

physx::PxTriangleMesh* PhysxRigidbody::GenerateTriangleMesh(std::string Filename, glm::vec3 scale)
{
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::FMeshLoadingSettings t;
	t.Scale = scale;
	MeshLoader::LoadMeshFromFile_Direct(Filename, t, vertices, indices);
	std::vector <glm::vec3> verts;
	for (int i = 0; i < vertices.size(); i++)
	{
		verts.push_back(vertices[i].m_position);
	}
	PxTriangleMeshDesc meshDesc = PxTriangleMeshDesc();
	meshDesc.points.count = (PxU32)verts.size();
	meshDesc.points.stride = sizeof(glm::vec3);
	meshDesc.points.data = verts.data();

	meshDesc.triangles.count = (PxU32)(indices.size() / 3);
	meshDesc.triangles.stride = 3 * sizeof(int);
	meshDesc.triangles.data = indices.data();

	ensure(meshDesc.isValid());
	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = PhysxEngine::GetCooker()->cookTriangleMesh(meshDesc, writeBuffer, &result);
	ensure(status);
	ensure(result == PxTriangleMeshCookingResult::eSUCCESS);
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	return PhysxEngine::GetGPhysics()->createTriangleMesh(readBuffer);
}
physx::PxConvexMesh* PhysxRigidbody::GenerateConvexMesh(std::string Filename, glm::vec3 scale)
{
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::FMeshLoadingSettings t;
	t.Scale = scale;
	MeshLoader::LoadMeshFromFile_Direct(Filename, t, vertices, indices);
	std::vector <glm::vec3> verts;
	for (int i = 0; i < vertices.size(); i++)
	{
		verts.push_back(vertices[indices[i]].m_position);
	}
	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = (PxU32)verts.size();
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = verts.data();
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
	convexDesc.vertexLimit = 10;

	PxDefaultMemoryOutputStream buf;
	bool status = PhysxEngine::GetCooker()->cookConvexMesh(convexDesc, buf);
	ensure(status);

	PxDefaultMemoryInputData readBuffer(buf.getData(), buf.getSize());
	return PhysxEngine::GetGPhysics()->createConvexMesh(readBuffer);
}
void PhysxRigidbody::AttachCollider(Collider * col)
{

	PMaterial = PhysxEngine::GetDefaultMaterial();
	PxShape* newShape = nullptr;
	for (int i = 0; i < col->Shapes.size(); i++)
	{
		ShapeElem* Shape = col->Shapes[i];
		if (Shape == nullptr)
		{
			Log::LogMessage("Invalid physx Shape", Log::Severity::Error);
			continue;
		}
		switch (Shape->GetType())
		{
		case EShapeType::eBOX:
		{
			BoxElem* BoxShape = (BoxElem*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxBoxGeometry(PhysxEngine::GLMtoPXvec3(BoxShape->Extents)), *PMaterial);
			break;
		}
		case EShapeType::eSPHERE:
		{
			SphereElem* SphereShape = (SphereElem*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxSphereGeometry(SphereShape->raduis), *PMaterial);
			break;
		}
		case EShapeType::eCAPSULE:
		{
			CapsuleElm* SphereShape = (CapsuleElm*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxCapsuleGeometry(SphereShape->raduis, SphereShape->height), *PMaterial);
			break;
		}
		case EShapeType::ePLANE:
		{
			PlaneElm* SphereShape = (PlaneElm*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxPlaneGeometry(), *PMaterial);
			newShape->setLocalPose(PxTransform(PhysxEngine::GLMtoPXvec3(glm::vec3(0, 0, 0)), PxQuat(0, 1, 0, 0)));
			break;
		}
		case EShapeType::eCONVEXMESH:
		{
			ConvexMeshElm* SphereShape = (ConvexMeshElm*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxConvexMeshGeometry(GenerateConvexMesh(SphereShape->MeshAssetName, SphereShape->Scale)), *PMaterial);
			break;
		}
		case EShapeType::eTRIANGLEMESH:
		{
			TriMeshElm* SphereShape = (TriMeshElm*)Shape;
			newShape = PhysxEngine::GetGPhysics()->createShape(PxTriangleMeshGeometry(GenerateTriangleMesh(SphereShape->MeshAssetName, SphereShape->Scale)), *PMaterial);
			break;
		}
		}
		ensure(newShape);
		newShape->userData = col;
		Shapes.push_back(newShape);
	}
	AttachedColliders.push_back(col);
	col->SetOwner(this);
}

void PhysxRigidbody::SetPhysicalMaterial(PhysicalMaterial * newmat)
{
	if (newmat != nullptr && newmat != PhysicsMat)
	{
		PhysicsMat = newmat;
		PMaterial = Engine::GetPhysEngineInstance()->CreatePhysxMat(PhysicsMat);
		for (int i = 0; i < Shapes.size(); i++)
		{
			Shapes[i]->setMaterials(&PMaterial, 1);
		}
	}
}

void PhysxRigidbody::UpdateBodyState()
{
	if (Dynamicactor)
	{
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, BodyData.LockXRot);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, BodyData.LockYRot);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, BodyData.LockZRot);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, BodyData.LockXPosition);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, BodyData.LockYPosition);
		Dynamicactor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, BodyData.LockZPosition);
		Dynamicactor->setLinearDamping(BodyData.LinearDamping);
		Dynamicactor->setAngularDamping(BodyData.AngularDamping);
	}
	if (CommonActorPtr)
	{
		CommonActorPtr->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !BodyData.Gravity);
	}
}

void PhysxRigidbody::InitBody()
{
	if (PhysicsMat != nullptr)
	{
		PMaterial = Engine::GetPhysEngineInstance()->CreatePhysxMat(PhysicsMat);
	}
	else
	{
		PhysicsMat = PhysicalMaterial::GetDefault();
	}

	if (BodyType == EBodyType::RigidDynamic)
	{
		Dynamicactor = PhysxEngine::GetGPhysics()->createRigidDynamic(PxTransform(PhysxEngine::GLMtoPXvec3(transform.GetPos()), PhysxEngine::GLMtoPXQuat(transform.GetQuatRot())));
		for (int i = 0; i < Shapes.size(); i++)
		{
			Dynamicactor->attachShape(*Shapes[i]);
		}
		Dynamicactor->setAngularDamping(BodyData.AngularDamping);
		Dynamicactor->setLinearDamping(BodyData.LinearDamping);
		if (BodyData.UseAutoMass)
		{
			Dynamicactor->setMass(BodyData.Mass);
		}
		else
		{
			PxRigidBodyExt::updateMassAndInertia(*Dynamicactor, PhysicsMat->density);
		}
		CommonActorPtr = Dynamicactor;
	}
	else if (BodyType == EBodyType::RigidStatic)
	{
		StaticActor = PhysxEngine::GetGPhysics()->createRigidStatic(PxTransform(PhysxEngine::GLMtoPXvec3(transform.GetPos()), PhysxEngine::GLMtoPXQuat(transform.GetQuatRot())));
		for (int i = 0; i < Shapes.size(); i++)
		{
			StaticActor->attachShape(*Shapes[i]);
		}
		CommonActorPtr = StaticActor;
	}
	UpdateBodyState();
	CommonActorPtr->userData = this;
	PhysxEngine::GetPlayScene()->addActor(*CommonActorPtr);
}
float PhysxRigidbody::GetMass()
{
	if (Dynamicactor == nullptr)
	{
		return 1.0f;
	}
	return Dynamicactor->getMass();
}
#endif