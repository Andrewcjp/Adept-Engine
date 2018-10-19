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
		TD_API float GetBodyMass();
		float GetInvBodyMass();
		virtual void AddForce(glm::vec3 Force, bool AsForce) {};
		virtual glm::vec3 GetVelocityDelta();
		virtual glm::vec3 GetLinearVelocity();
		virtual void SetLinearVelocity(glm::vec3 newvel);
		virtual void ResetForceThisFrame() {};
		virtual void AttachShape(TDShape* newShape);
		std::vector<TDShape*>& GetAttachedShapes();

	protected:
		TD_API virtual ~TDActor();
		float BodyMass = 1.0f;
		glm::vec3 LinearVelocity = glm::vec3();
		TDTransform Transform;
		TDActorType::Type ActorType;
		TDScene* OwningScene = nullptr;
		friend class TDScene;
		std::vector<TDShape*> AttachedShapes;
	};
}
