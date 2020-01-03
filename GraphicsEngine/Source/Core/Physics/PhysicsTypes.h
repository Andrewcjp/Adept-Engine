#pragma once
#include "Physics/Physics_fwd.h"
namespace EShapeType
{
	enum Type
	{
		eSPHERE,
		ePLANE,
		eCAPSULE,
		eBOX,
		eCONVEXMESH,
		eTRIANGLEMESH,
		eHeightField,
		eLimit
	};
}
namespace EForceMode
{
	enum Type
	{
		AsAcceleration,
		AsForce,
		Limit,
	};
}
namespace EBodyType
{
	enum Type
	{
		RigidStatic,
		RigidDynamic,
		Limit
	};
}
class GameObject;
typedef struct _RayHit
{
	glm::vec3 position = glm::vec3();
	RigidBody* HitBody = nullptr;
	GameObject* HitObject = nullptr;
	bool StartInside = false;
	float Distance = 0.0f;
	glm::vec3 Normal = glm::vec3();
}RayHit;
struct ShapeElem
{
	ShapeElem()
	{};
	EShapeType::Type GetType()
	{
		return ShapeType;
	}
protected:
	EShapeType::Type ShapeType = EShapeType::eLimit;
};
struct BoxElem :public ShapeElem
{
	glm::vec3 Extents;
	BoxElem()
	{
		ShapeType = EShapeType::eBOX;
	};
};
struct SphereElem :public ShapeElem
{
	float raduis = 1;
	SphereElem()
	{
		ShapeType = EShapeType::eSPHERE;
	};
};
struct PlaneElm :public ShapeElem
{
	glm::vec3 scale = glm::vec3(0, 1, 0);
	PlaneElm()
	{
		ShapeType = EShapeType::ePLANE;
	};
};
struct TriMeshElm :public ShapeElem
{
	std::string MeshAssetName = "";
	glm::vec3 Scale = glm::vec3(1);
	TriMeshElm()
	{
		ShapeType = EShapeType::eTRIANGLEMESH;
	};
};
struct ConvexMeshElm :public ShapeElem
{
	std::string MeshAssetName = "";
	glm::vec3 Scale = glm::vec3(1);
	ConvexMeshElm()
	{
		ShapeType = EShapeType::eCONVEXMESH;
	};
};
struct CapsuleElm :public ShapeElem
{
	float height = 2;
	float raduis = 1;
	CapsuleElm()
	{
		ShapeType = EShapeType::eCAPSULE;
	};
};
struct CollisonData
{
	Collider* Hitcollider = nullptr;
	Collider* OtherCollider = nullptr;
};
struct PhysicalMaterial
{
	float StaticFriction = 0.5f;
	float DynamicFirction = 0.6f;
	float Bouncyness = 0.6f;
	float density = 1.0;
	static PhysicalMaterial* GetDefault()
	{
		return new PhysicalMaterial();
	}
};

