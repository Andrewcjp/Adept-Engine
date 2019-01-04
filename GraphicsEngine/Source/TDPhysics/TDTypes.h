#pragma once
#include "TDPCH.h"
namespace TD
{
	class TDShape;
	class TDActor;
	class TDQuerryFilter;
	const int MAX_CONTACT_POINTS_COUNT = 50;
	struct TD_API TDPhysicalMaterial
	{
		TDPhysicalMaterial();
		float StaticFriction = 0.6f;
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
		TDShape* Shape = nullptr;
	};
	struct RaycastData
	{
		bool BlockingHit = false;
		bool StartPenetrating = false;
		Contact Points[MAX_CONTACT_POINTS_COUNT];
		int Count = 0;
		void AddContact(glm::vec3 Point, glm::vec3 Normal, float Depth, TDShape* shape);
		void SortClosest();
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
			eAABB,
			eLimit
		};
	}

	namespace TDForceMode
	{
		enum Type
		{
			AsAcceleration,
			AsForce,
			AsVelocityChange,
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
	/// Broadphase Method
	namespace TDBroadphaseMethod
	{
		enum Type
		{
			SAP,///< Sweep and Prune
			HSAP,///< Hierarchical Sweep and Prune
			MBP,///< Multi Box Pruning
			BFBE,///< Brute force Box elimination  
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
		RayCast(glm::vec3 origin, glm::vec3 dir, float distance, RaycastData* hitData, TDQuerryFilter* Filter)
		{
			Origin = origin;
			Dir = dir;
			Distance = distance;
			HitData = hitData;
			InterSectionFilter = Filter;
		}
		void Validate();
		glm::vec3 Origin;
		glm::vec3 Dir;
		float Distance;
		RaycastData* HitData = nullptr;
		TDQuerryFilter* InterSectionFilter = nullptr;
		bool PreFilter(TDActor* actor, TDShape* Shape);
		bool PostFilter();
	};
	///Holds the collision data about two shapes
	struct ShapeCollisionPair
	{
		ShapeCollisionPair(TDShape* A, TDShape* B);
		TDShape* A = nullptr;
		TDActor* AOwner = nullptr;
		TDShape* B = nullptr;
		TDActor* BOwner = nullptr;
		ContactData Data;
		bool IsTriggerPair = false;
		///Does this pair simulate physics?
		bool SimPair = false;
		bool IsPairValidForTrigger();
	};
	///Holds the collision data about Two actors and their shapes in ShapeCollisionPairs
	struct ActorCollisionPair
	{
		ActorCollisionPair(TDActor* A, TDActor* B);
		TDActor* first = nullptr;
		TDActor* second = nullptr;
		void CreateShapePairs();
		bool operator==(const ActorCollisionPair& rhs)//order matters - for SAP
		{
			return (this->first == rhs.first && this->second == rhs.second);
		}
		std::vector<ShapeCollisionPair> ShapePairs;
		void Reset();
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
		glm::vec3 Offset;
		float RestLength = 5.0f;
		float Dampening = 0.1f;
		float SpringK = 1.0f;
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
			ETrigger = (1 << 1),
			Limit = (1 << 1),
		};
	};
	class TD_API TDActorFlags : public TDFlagsBase
	{
	public:
		enum
		{
			EKinematic = (1 << 0),
			ELockPosX = (1 << 1),
			ELockPosY = (1 << 2),
			ELockPosZ = (1 << 3),
			ELockRotX = (1 << 4),
			ELockRotY = (1 << 5),
			ELockRotZ = (1 << 6),
			Limit = (1 << 1),
		};
	};
	struct ContactPair
	{
		TDShape* ShapeA = nullptr;
		TDShape* ShapeB = nullptr;
		ContactPair()
		{};
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
	typedef void(*CollisionFilterFunction)(TDShape* A, TDShape* B);
};