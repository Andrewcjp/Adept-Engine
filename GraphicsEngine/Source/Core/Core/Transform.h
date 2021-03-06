#pragma once
#include "Rendering/Core/Camera.h"
#include "Types/WeakObjectPtr.h"
#include "GameObject.h"
#define USE_TRANSFORM_CACHING 0
class Transform
{
public:
	CORE_API Transform(const glm::vec3& pos = glm::vec3(), const glm::vec3& rot = glm::vec3(), const glm::vec3& scale = glm::vec3(1, 1, 1));

	CORE_API glm::mat4 GetModel(bool NoParent = false);
	glm::mat4 GetMVP(Camera* camera);
	glm::mat4 GetTransFormView();
	///------------------------------------------------------
	//setters	
	CORE_API void SetPos(const glm::vec3& pos);
	CORE_API void SetEulerRot(const glm::vec3& rot);
	CORE_API void SetScale(const glm::vec3& scale);
	CORE_API void AddRotation(glm::vec3& rot);
	CORE_API void RotateAboutAxis(glm::vec3& axis, float amt);
	CORE_API void SetQrot(const glm::quat& val);
	void SetParent(GameObject* Parent);
	void TranslatePos(const glm::vec3 & pos);
	void MakeRotationFromXY(const glm::vec3 & Fwd, const glm::vec3 & up);
	CORE_API void SetLocalPosition(glm::vec3 localpos);
	///------------------------------------------------------
	//getters
	CORE_API glm::vec3 GetPos();
	CORE_API glm::vec3 GetEulerRot();
	CORE_API glm::vec3 GetScale();
	CORE_API glm::vec3 GetForward();
	CORE_API glm::vec3 GetUp();
	CORE_API glm::vec3 GetRight();
	glm::vec3 TransfromToLocalSpace(glm::vec3& direction);
	glm::vec3 TransformDirection(const glm::vec3& pDirection, const  glm::mat4& pMatrix);
	CORE_API glm::quat GetQuatRot();
	CORE_API Transform* GetParent()const;
	bool IsChanged();
	void Update();
	void Serilise(class Archive* A);
	CORE_API void SetLocalRotation(glm::quat localrot);
	void Translate(glm::vec3 direction, float amt);
	void Set(glm::mat4 materix);
	bool Slave = false;
	uint GetLastMovedFrame()const;
private:
	uint LastChangedFrame = 0;
	//cached matrixs;
	glm::mat4 CacheModel;
	bool UpdateModel = true;

	glm::vec3 _pos;
	glm::vec3 _rot;
	glm::vec3 _scale;
	glm::quat _qrot;
	glm::mat4 LocalModelMatrix;
	glm::mat4 parentMatrix;
	WeakObjectPtr<GameObject> parent;

	glm::vec3 oldpos = glm::vec3();
	glm::vec3 oldscale = glm::vec3();
	glm::quat oldqrot = glm::vec3();
};

