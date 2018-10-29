#pragma once
#include "TDPCH.h"
namespace TD
{
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
	struct RaycastData
	{
		bool BlockingHit = false;
		bool StartPenetrating = false;
		float Distance = 0.0f;
		glm::vec3 Point = glm::vec3();
		glm::vec3 Normal = glm::vec3();
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
	namespace TDPerfCounters
	{
		enum Type
		{
			ResolveCollisions,
			IntergrateScene,
			ResolveConstraints,
			IntersectionTests,
			eLimit
		};
	}
};