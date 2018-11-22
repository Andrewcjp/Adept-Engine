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
		TD_API glm::vec3 GetLinearVelocity();
		TD_API void SetLinearVelocity(glm::vec3 newvel);
		TD_API void AddForce(glm::vec3 Force, bool AsForce);

		

		//Angular Motion		
		TD_API glm::mat4x4 GetInertiaTensor();
		TD_API void SetInertiaTensor(glm::mat4x4 tensor);
		TD_API glm::vec3 GetAngularVelocity();
		
		TD_API void SetAngularVelocity(glm::vec3 velocity);

		TD_API void AddTorque(glm::vec3 Torque);
		TD_API void AddForceAtPosition(glm::vec3 pos, glm::vec3 force);
		//Internal Functions
		/**\brief Internal: the sum of velocity change this frame*/
		glm::vec3 GetLinearVelocityDelta();
		glm::vec3 GetAngularVelocityDelta();
		void ResetForceThisFrame();
		void UpdateSleepTimer(float DT);
		bool IsBodyAsleep()const;
		bool IsAffectedByGravity()const { return UseGravity; };
	private:
		void ComputeInertiaTensor();
		bool CheckSleep(glm::vec3 & value);
		glm::vec3 DeltaLinearVel = glm::vec3();
		glm::vec3 DeltaAngularVel = glm::vec3();
		bool UseGravity = true;
		//RigidBody Physics Values
		//Linear
		float BodyMass = 1.0f;
		glm::vec3 LinearVelocity = glm::vec3();
		//Angular
		glm::vec3 AngularVel = glm::vec3();
		glm::mat4x4 InertaTensor;
		float CachedsqSleepZeroThreshold = 0.0f;
		float SleepTimer = 0.0f;
		float SleepMaxTimer = 10.0f;
	};

}