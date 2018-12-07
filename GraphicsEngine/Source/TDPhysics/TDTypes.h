#pragma once

namespace TD
{
	class TDShape;
	class TDActor;
	const int MAX_CONTACT_POINTS_COUNT = 50;
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
	struct Contact
	{
		float Distance = 0.0f;
		glm::vec3 Point = glm::vec3();
		glm::vec3 Normal = glm::vec3();
	};
	struct RaycastData
	{
		bool BlockingHit = false;
		bool StartPenetrating = false;
		Contact Points[MAX_CONTACT_POINTS_COUNT];
		int Count = 0;
		void AddContact(glm::vec3 Point, glm::vec3 Normal, float Depth);
		void Reset();
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
	
	struct ContactData
	{
		glm::vec3 ContactPoints[MAX_CONTACT_POINTS_COUNT];
		bool Blocking = false;
		glm::vec3 Direction[MAX_CONTACT_POINTS_COUNT];
		float depth[MAX_CONTACT_POINTS_COUNT];
		int ContactCount = 0;
		void Contact(glm::vec3 position, glm::vec3 normal, float seperation);
		void Reset();
	};

	struct RayCast
	{
		RayCast(glm::vec3 origin, glm::vec3 dir, float distance, RaycastData* hitData)
		{
			Origin = origin;
			Dir = dir;
			Distance = distance;
			HitData = hitData;
		}
		glm::vec3 Origin;
		glm::vec3 Dir;
		float Distance;
		RaycastData* HitData = nullptr;
	};

	struct CollisionPair
	{
		CollisionPair(TDActor* A, TDActor* B);
		TDActor* first = nullptr;
		TDActor* second = nullptr;
		ContactData data;
		bool IsTriggerPair = false;
		bool operator==(const CollisionPair& rhs)
		{
			return (this->first == rhs.first && this->second == rhs.second) || (this->first == rhs.second && this->first == rhs.second);
		}
	};

	namespace TDConstraintType
	{
		enum Type
		{
			Distance,
			Spring,
			Motor,
			Limit
		};
	}

	struct ConstraintDesc
	{
		float distance = 0.0f;
		TDConstraintType::Type Type;
	};

	class TD_API TDFlagsBase
	{
	public:
		void SetFlagValue(int flag, bool value);
		bool GetFlagValue(int flag);
		void SetFlags(int flags);
	private:
		int Flags = 0;
	};

	class TD_API TDShapeFlags :public TDFlagsBase
	{
	public:
		enum Type
		{
			ESimulation = (1 << 0),
			ETrigger = (2 << 0),
			Limit = (1 << 1),
		};
	};

	struct ContactPair
	{
		TDShape* ShapeA = nullptr;
		TDShape* ShapeB = nullptr;
		ContactPair() {};
		ContactPair(TDShape* a, TDShape* b)
		{
			ShapeA = a;
			ShapeB = b;
		}
		bool operator==(const ContactPair* A)
		{
			return this->ShapeA == A->ShapeA && this->ShapeB == A->ShapeB || (this->ShapeB == A->ShapeA && this->ShapeA == A->ShapeB);
		}
		bool operator==(const ContactPair& A)
		{
			return this->ShapeA == A.ShapeA && this->ShapeB == A.ShapeB || (this->ShapeB == A.ShapeA && this->ShapeA == A.ShapeB);
		}
	};

};