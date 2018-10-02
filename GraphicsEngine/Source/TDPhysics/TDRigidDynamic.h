#pragma once
#include "TDActor.h"
namespace TD
{
	class TDRigidDynamic : public TDActor
	{
	public:
		TD_API TDRigidDynamic();
		~TDRigidDynamic();
		virtual glm::vec3 GetVelocityDelta() override;
		void AddForce(glm::vec3 Force, bool AsForce);
		void ResetForceThisFrame()override;
	private:
		glm::vec3 DeltaVel = glm::vec3();
	};

}