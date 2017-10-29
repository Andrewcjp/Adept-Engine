#pragma once
#include "include\glm\glm.hpp"
#include "include\glm\gtx\transform.hpp"
#include "include\glm\gtc\quaternion.hpp"
#include "include\glm\gtx\quaternion.hpp"

#include "include/glm/ext.hpp"
#include "../Rendering/Core/Camera.h"
class Transform
{
public:
	Transform(const glm::vec3& pos = glm::vec3(), const glm::vec3& rot = glm::vec3(), const glm::vec3& scale = glm::vec3(1, 1, 1)) :
		_pos(pos),
		_qrot(glm::quat(rot)),
		_scale(scale) {
		parentMatrix = glm::mat4(1);//Identity
	}
	inline glm::mat4 GetModel()
	{
		glm::mat4 posMat = glm::translate(_pos);
		glm::mat4 scaleMat = glm::scale(_scale);

		glm::mat4 rotMat = (glm::toMat4(((_qrot))));
		//todo:could optimise!
		if (parent != nullptr && parent->IsChanged()) {
			parentMatrix = parent->GetModel();
			oldpos = _pos;
			oldqrot = _qrot;
			oldscale = _scale;
		}

		return  posMat *  rotMat * scaleMat;
	}
	inline glm::mat4 GetMVP(Camera* camera)
	{
		glm::mat4 VP = camera->GetViewProjection();
		glm::mat4 M = GetModel();

		return VP * M;
	}

	inline glm::mat4 GetTransFormView() {
		return glm::lookAt(GetPos(), GetPos() + GetForward(), GetUp());
	}
	inline glm::vec3 GetPos() const { return _pos; }
	inline glm::vec3 GetEulerRot() const { return _rot; }
	inline glm::vec3 GetScale() const { return _scale; }

	inline void SetPos(glm::vec3& pos) { this->_pos = pos; }
	inline void SetEulerRot(glm::vec3& rot) {
		this->_qrot = glm::quat(glm::radians(rot));
	}
	inline void SetScale(glm::vec3& scale) { this->_scale = scale; }
	inline void AddRotation(glm::vec3& rot) {
		this->_rot += rot;
	}
	inline void RotateAboutAxis(glm::vec3& axis, float amt) {
		this->_qrot *= glm::angleAxis((amt), glm::normalize(axis));
	}
	inline glm::vec3 GetForward() {
		return TransformDirection(glm::vec3(0, 0, 1), GetModel());
	}
	inline glm::vec3 GetUp() {
		return TransformDirection(glm::vec3(0, 1, 0), GetModel());
	}
	glm::vec3 TransfromToLocalSpace(glm::vec3& direction) {
		return TransformDirection(direction, GetModel());
	}
	glm::vec3 TransformDirection(glm::vec3& pDirection, glm::mat4& pMatrix) {
		return glm::normalize(glm::vec3(pMatrix * glm::vec4(pDirection, 0.0f)));
	}
	glm::quat GetQuatRot()const {
		return _qrot;
	}
	inline void SetQrot(glm::quat& val) {
		_qrot = val;
	}
	inline void SetParent(Transform* Parent) {
		parent = Parent;
	}
	Transform* GetParent()const {
		return parent;
	}
	bool IsChanged() {

		if (oldpos != _pos)
			return true;

		if (oldqrot != _qrot)
			return true;

		if (oldscale != _scale)
			return true;

		return false;
	}
	inline void TranslatePos(glm::vec3 & pos) {
		_pos += pos;
	}
private:
	glm::vec3 _pos;
	glm::vec3 _rot;
	glm::vec3 _scale;
	glm::quat _qrot;
	glm::mat4 parentMatrix;
	Transform* parent;
	glm::vec3 oldpos;
	glm::vec3 oldscale;
	glm::quat oldqrot;
};

