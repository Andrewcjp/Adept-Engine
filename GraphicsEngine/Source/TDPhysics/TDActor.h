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
		TD_API TDTransform* GetTransfrom();
		float GetBodyMass();
		virtual void AddForce(glm::vec3 Force, bool AsForce) {};
		virtual glm::vec3 GetVelocityDelta();
		virtual glm::vec3 GetLinearVelocity();
		virtual void SetLinearVelocity(glm::vec3 newvel);
		virtual void ResetForceThisFrame() {};
	protected:
		~TDActor();
		float BodyMass = 1.0f;
		glm::vec3 LinearVelocity = glm::vec3();
		TDTransform Transform;
		TDActorType::Type ActorType;
		TDScene* OwningScene = nullptr;
		friend class TDScene;
	};
}
