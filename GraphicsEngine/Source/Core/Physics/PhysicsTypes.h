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
		eLimit
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
typedef struct _RayHit
{
	glm::vec3 position = glm::vec3();
	RigidBody* HitBody = nullptr;
	bool StartInside = false;
}RayHit;
struct ShapeElem
{
	ShapeElem() {};
	EShapeType::Type GetType() { return ShapeType; }
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

