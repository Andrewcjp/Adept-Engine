#pragma once
#include "TDActor.h"
namespace TD
{
	class TDRigidDynamic : public TDActor
	{
	public:
		TD_API TDRigidDynamic();
		~TDRigidDynamic();		
		
		TD_API void SetGravity(bool enabled);
		TD_API bool IsEffectedByGravity();
		/**\return The bodies mass in Kg*/
		TD_API float GetBodyMass();
		/**\returns The inverse bodies mass in Kg, if mass is zero, zero is returned */
		float GetInvBodyMass();
		//Linear Motion
		glm::vec3 GetLinearVelocity();
		void SetLinearVelocity(glm::vec3 newvel);
		TD_API void AddForce(glm::vec3 Force, bool AsForce);
		//Angular Motion
		glm::vec3 GetAngularVelocity();
		void SetAngularVelocity(glm::vec3 velocity);
		void AddTorque(glm::vec3 Torque);

		//Internal Functions
		/**\brief Internal: the sum of velocity change this frame*/
		glm::vec3 GetLinearVelocityDelta();
		glm::vec3 GetAngularVelocityDelta();
		void ResetForceThisFrame();
	private:
		glm::vec3 DeltaLinearVel = glm::vec3();
		glm::vec3 DeltaAngularVel = glm::vec3();
		bool UseGravity = false;
		//RigidBody Physics Values
		//Linear
		float BodyMass = 1.0f;
		glm::vec3 LinearVelocity = glm::vec3();
		//Angular
		glm::vec3 AngularVel = glm::vec3();
		float InertaTensor = 1.0f;
	};

}