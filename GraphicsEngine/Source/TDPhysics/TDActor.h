#pragma once
#include "TDTransform.h"
namespace TD
{
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
	class TDActor
	{
	public:
		TDActor();
		virtual void Init();
		TDActorType::Type GetActorType() const;
		TDScene* GetScene() const;
		void Release();
		TDTransform* GetTransfrom();
	protected:
		~TDActor();
		TDTransform Transform;
		TDActorType::Type ActorType;
		TDScene* OwningScene = nullptr;
		friend class TDScene;
	};
}
