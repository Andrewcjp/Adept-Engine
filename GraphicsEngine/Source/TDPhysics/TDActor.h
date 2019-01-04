#pragma once
#include "TDTransform.h"
#include "TDTypes.h"

namespace TD
{
	class TDRigidDynamic;
}
namespace TD
{
	class TDAABB;
	class TDSphere;
	class TDScene;
	struct TDActorType
	{
		enum Type
		{
			RigidStatic,
			RigidDynamic,
			Limit
		};
	};
	class TDShape;
	class TDActor
	{
	public:
		TD_API TDActor();
		TD_API virtual ~TDActor();
		virtual void Init();
		TDActorType::Type GetActorType() const;
		TDScene* GetScene() const;
		void Release();
		TD_API TDTransform* GetTransfrom();
		virtual void AttachShape(TDShape* newShape);
		TD_API std::vector<TDShape*>& GetAttachedShapes();
		void UpdateTransfrom();
		template<class T>
		static T* ActorCast(TDActor* actor)
		{
			return dynamic_cast<T*>(actor);
		}
		///Special case function to handle kinematics 
		static TDRigidDynamic* RigidCast(TDActor* actor);
		/// The AABB used for the Broadphase Representation of this object 
		TDAABB* AABB = nullptr;
		///User Defined Data - Normally used to link to engine object of this shape.
		void* UserData = nullptr;
		void UpdateAABBPos(glm::vec3 pos);
		void SetAABBLocalPos(glm::vec3 localpos);
		TD_API TDActorFlags& GetFlags();
#if VALIDATE_KE
		virtual void ComputeKE();
		virtual void ValidateKE();
		float PreSimKE = 0.0f;
#endif
	protected:
		TDScene* OwningScene = nullptr;
		friend class TDScene;
		std::vector<TDShape*> AttachedShapes;
		TDActorType::Type ActorType;
		TDTransform Transform;
		glm::vec3 LocalAABBPos;
		TDActorFlags ActorFlags;

	};
}
