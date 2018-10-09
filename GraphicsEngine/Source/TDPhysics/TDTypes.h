#pragma once
namespace TD {
	struct TDPhysicalMaterial
	{
		float StaticFriction = 0.5f;
		float DynamicFirction = 0.6f;
		float Bouncyness = 0.6f;
		float density = 1.0;
		static TDPhysicalMaterial* GetDefault()
		{
			return new TDPhysicalMaterial();
		}
	};
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
};