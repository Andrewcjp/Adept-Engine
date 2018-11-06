#pragma once

namespace TD
{
	struct TD_API TDPhysicalMaterial
	{
		TDPhysicalMaterial();
		float StaticFriction = 0.2f;
		float DynamicFirction = 0.6f;
		float Restitution = 0.2f;
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
	namespace TDBroadphaseMethod
	{
		enum Type
		{
			SAP,
			HSAP,
			MBP,
			Limit
		};
	}
	const int MAX_CONTACT_POINTS_COUNT = 50;
	struct ContactData
	{
		glm::vec3 ContactPoints[MAX_CONTACT_POINTS_COUNT];
		bool Blocking = false;
		glm::vec3 Direction = glm::vec3();
		float depth = 0.0f;
		int ContactCount = 0;
		void Contact(glm::vec3 position, glm::vec3 normal, float seperation)
		{
			Blocking = true;
			ContactPoints[ContactCount] = position;
			ContactCount++;
			Direction = glm::normalize(normal);
			depth = seperation;
		}
		void Reset();
	};
	class TDActor;
	struct CollisionPair
	{
		CollisionPair(TDActor* A, TDActor* B)
		{
			first = A;
			second = B;
		}
		TDActor* first = nullptr;
		TDActor* second = nullptr;
		ContactData data;
	};
};