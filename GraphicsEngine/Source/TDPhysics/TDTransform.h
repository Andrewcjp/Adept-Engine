#pragma once
#include "include\glm\glm.hpp"
#include "include\glm\gtx\transform.hpp"
#include "include\glm\gtc\quaternion.hpp"
#include "include\glm\gtx\quaternion.hpp"
#include "include/glm/ext.hpp"
namespace TD
{
	class TDTransform
	{
	public:
		TD_API TDTransform(const glm::vec3& pos = glm::vec3(), const glm::vec3& rot = glm::vec3(), const glm::vec3& scale = glm::vec3(1, 1, 1));

		//------------------------------------------------------
		//setters	
		TD_API void SetPos(const glm::vec3& pos);
		TD_API void SetEulerRot(const glm::vec3& rot);
		TD_API void SetScale(const glm::vec3& scale);

		void AddRotation(glm::vec3& rot);
		void RotateAboutAxis(glm::vec3& axis, float amt);
		TD_API void SetQrot(const glm::quat& val);
		void SetParent(TDTransform* Parent);
		void TranslatePos(const glm::vec3 & pos);
		void MakeRotationFromXY(const glm::vec3 & Fwd, const glm::vec3 & up);
		glm::mat4 GetModel();
		//------------------------------------------------------
		//getters
		TD_API glm::vec3 GetPos() ;
		TD_API glm::vec3 GetEulerRot() const;
		TD_API glm::vec3 GetScale() const;
		void SetLocalPosition(glm::vec3 localpos);
		glm::vec3 GetForward();
		glm::vec3 GetUp();
		glm::vec3 GetRight();
		glm::vec3 TransfromToLocalSpace(glm::vec3& direction);
		glm::vec3 TransformDirection(const glm::vec3& pDirection, const  glm::mat4& pMatrix)const;
		TD_API glm::quat GetQuatRot()const;
		TDTransform* GetParent()const;
		bool IsChanged();
		void Update();
	private:
		//cached matrixs;
		glm::mat4 CacheModel;
		glm::mat4 CacheLocalModel;
		bool UpdateModel = true;

		glm::vec3 _pos;
		glm::vec3 _rot;
		glm::vec3 _scale;
		glm::quat _qrot;
		glm::mat4 parentMatrix;
		TDTransform* parent = nullptr;

		glm::vec3 oldpos = glm::vec3();
		glm::vec3 oldscale = glm::vec3();
		glm::quat oldqrot = glm::vec3();
	};

}