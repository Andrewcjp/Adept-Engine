#pragma once
#include "include\glm\glm.hpp"
#include "include\glm\gtx\transform.hpp"
#include "include\glm\gtc\quaternion.hpp"
#include "include\glm\gtx\quaternion.hpp"
#include "include/glm/ext.hpp"
#include "Rendering/Core/Camera.h"
class Transform
{
public:
	CORE_API Transform(const glm::vec3& pos = glm::vec3(), const glm::vec3& rot = glm::vec3(), const glm::vec3& scale = glm::vec3(1, 1, 1));

	glm::mat4 GetModel();
	glm::mat4 GetMVP(Camera* camera);
	glm::mat4 GetTransFormView();
	///------------------------------------------------------
	//setters	
	CORE_API void SetPos(const glm::vec3& pos);
	CORE_API void SetEulerRot(const glm::vec3& rot);
	CORE_API void SetScale(const glm::vec3& scale);
	CORE_API void AddRotation(glm::vec3& rot);
	CORE_API void RotateAboutAxis(glm::vec3& axis, float amt);
	void SetQrot(const glm::quat& val);
	void SetParent(Transform* Parent);
	void TranslatePos(const glm::vec3 & pos);
	void MakeRotationFromXY(const glm::vec3 & Fwd, const glm::vec3 & up);
	///------------------------------------------------------
	//getters
	glm::vec3 GetPos() const { return _pos; }
	CORE_API glm::vec3 GetEulerRot() const;
	glm::vec3 GetScale() const { return _scale; }
	CORE_API glm::vec3 GetForward();
	CORE_API glm::vec3 GetUp();
	CORE_API glm::vec3 GetRight();
	glm::vec3 TransfromToLocalSpace(glm::vec3& direction);
	glm::vec3 TransformDirection(const glm::vec3& pDirection, const  glm::mat4& pMatrix);
	glm::quat GetQuatRot()const;
	Transform* GetParent()const;
	bool IsChanged();
	void Update();
	void Serilise(class Archive* A);
private:
	//cached matrixs;
	glm::mat4 CacheModel;
	bool UpdateModel = true;

	glm::vec3 _pos;
	glm::vec3 _rot;
	glm::vec3 _scale;
	glm::quat _qrot;
	glm::mat4 parentMatrix;
	Transform* parent = nullptr;

	glm::vec3 oldpos =  glm::vec3();
	glm::vec3 oldscale = glm::vec3();
	glm::quat oldqrot = glm::vec3();
};

