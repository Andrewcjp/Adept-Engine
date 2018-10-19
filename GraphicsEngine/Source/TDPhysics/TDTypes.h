#pragma once
namespace TD {
	struct TDPhysicalMaterial
	{
		float StaticFriction = 0.5f;
		float DynamicFirction = 0.6f;
		float Restitution = 0.6f;
		float density = 1.0;
		static TDPhysicalMaterial* GetDefault()
		{
			return new TDPhysicalMaterial();
		}
	};
	namespace TDShapeType
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
	namespace TDForceMode
	{
		enum Type
		{
			AsAcceleration,
			AsForce,
			Limit,
		};
	}
	namespace TDBodyType
	{
		enum Type
		{
			RigidStatic,
			RigidDynamic,
			Limit
		};
	}
};