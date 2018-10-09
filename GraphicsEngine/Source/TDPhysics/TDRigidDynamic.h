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
		TD_API void AddForce(glm::vec3 Force, bool AsForce);
		void ResetForceThisFrame()override;
		TD_API void SetGravity(bool enabled);
		TD_API bool IsEffectedByGravity(); 
	private:
		glm::vec3 DeltaVel = glm::vec3();
		bool UseGravity = false;
	};

}