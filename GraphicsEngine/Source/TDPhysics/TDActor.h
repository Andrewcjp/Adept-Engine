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
		virtual void Init();
		TDActorType::Type GetActorType() const;
		TDScene* GetScene() const;
		void Release();
		TD_API TDTransform* GetTransfrom();
		virtual void AttachShape(TDShape* newShape);
		std::vector<TDShape*>& GetAttachedShapes();
		template<class T>
		static T* ActorCast(TDActor* actor)
		{
			return dynamic_cast<T*>(actor);
		}
		TDAABB* AABB = nullptr;
		virtual void ComputeKE();
		virtual void ValidateKE();
		void* UserData = nullptr;
	protected:
		float PreSimKE = 0.0f;
		TD_API virtual ~TDActor();
		TDScene* OwningScene = nullptr;
		friend class TDScene;
		std::vector<TDShape*> AttachedShapes;
		TDActorType::Type ActorType;
		TDTransform Transform;

	};
}
