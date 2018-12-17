#pragma once
#include "TDActor.h"
#include "TDTypes.h"
namespace TD
{
	class TDRigidDynamic : public TDActor
	{
	public:
		TD_API					TDRigidDynamic();
		TD_API					~TDRigidDynamic();

		TD_API void				SetGravity(bool enabled);

		TD_API bool				IsEffectedByGravity();
		/**\return The bodies mass in Kg*/
		TD_API float			GetBodyMass();
		/**\returns The inverse bodies mass in Kg, if mass is zero, zero is returned */
		TD_API float			GetInvBodyMass();

		//Linear Motion
		//**\returns the bodies current Linear Velocity 
		TD_API glm::vec3		GetLinearVelocity();
		///Sets the bodies Linear Velocity
		TD_API void				SetLinearVelocity(glm::vec3 newvel);
		///Add a force to this Body in Newtons 
		TD_API void				AddForce(glm::vec3 Force, TDForceMode::Type ForceMode);

		//Angular Motion		
		//**\returns a 4x4 Matrix containing the Inertia Tensor 
		TD_API glm::mat4x4		GetInertiaTensor();
		///Sets the Inertia Tensor For this body
		TD_API void				SetInertiaTensor(glm::mat4x4 tensor);
		//**\returns the Angular velocity in Rads/Second
		TD_API glm::vec3		GetAngularVelocity();

		TD_API void				SetAngularVelocity(glm::vec3 velocity);

		TD_API void				AddTorque(glm::vec3 Torque);
		TD_API void				AddForceAtPosition(glm::vec3 pos, glm::vec3 force);

		TD_API void				SetBodyMass(float Mass);
		///Internal Functions
		/**\brief Internal: the sum of velocity change this frame*/
		glm::vec3				GetLinearVelocityDelta();
		/**\brief Internal: the sum of angular velocity change this frame*/
		glm::vec3				GetAngularVelocityDelta();
		/**\brief Internal: for the solver to reset the bodies Delta V*/
		void					ResetForceThisFrame();
		/**\brief Internal: Tick the sleep timer*/
		void					UpdateSleepTimer(float DT);
		/**\returns true if the body is asleep*/
		TD_API bool				IsBodyAsleep()const;

		TD_API bool				IsAffectedByGravity()const { return UseGravity; };
		TD_API void				SetLinearDamping(float amt);
		TD_API float			GetLinearDamping()const;
#if VALIDATE_KE
		float					Compute_KE();
		void					ComputeKE();
		void					ValidateKE();
#endif
	private:
		void					ComputeInertiaTensor();
		bool					CheckSleep(glm::vec3 & value);
		glm::vec3				DeltaLinearVel = glm::vec3();
		glm::vec3				DeltaAngularVel = glm::vec3();
		bool					UseGravity = true;
		//RigidBody Physics Values
		//Linear
		float					BodyMass = 1.0f;
		glm::vec3				LinearVelocity = glm::vec3();
		//Angular
		glm::vec3				AngularVel = glm::vec3();
		glm::mat4x4				InertaTensor;
		float					CachedsqSleepZeroThreshold = 0.0f;
		float					SleepTimer = 0.0f;
		float					SleepMaxTimer = 10.0f;
		float					LinearDamping = 0.05f;
	};

}