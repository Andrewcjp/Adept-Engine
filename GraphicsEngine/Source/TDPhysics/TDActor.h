#pragma once
#include "TDTransform.h"
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
		std::vector<TDShape*>& GetAttachedShapes();
		void UpdateTransfrom();
		template<class T>
		static T* ActorCast(TDActor* actor)
		{
			return dynamic_cast<T*>(actor);
		}
		TDAABB* AABB = nullptr;
#if VALIDATE_KE
		virtual void ComputeKE();
		virtual void ValidateKE();
#endif
		void* UserData = nullptr;
		void UpdateAABBPos(glm::vec3 pos);
		void SetAABBLocalPos(glm::vec3 localpos) { LocalAABBPos = localpos; }
	protected:
		float PreSimKE = 0.0f;
		
		TDScene* OwningScene = nullptr;
		friend class TDScene;
		std::vector<TDShape*> AttachedShapes;
		TDActorType::Type ActorType;
		TDTransform Transform;
		glm::vec3 LocalAABBPos;

	};
}
