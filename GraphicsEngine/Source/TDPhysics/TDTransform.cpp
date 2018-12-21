#include "TDTransform.h"
#include "Utils/MathUtils.h"
namespace TD
{

	TDTransform::TDTransform(const glm::vec3 & pos, const glm::vec3 & rot, const glm::vec3 & scale) :
		_pos(pos),
		_qrot(glm::quat(rot)),
		_scale(scale)
	{
		parentMatrix = glm::mat4(1);//Identity
	}
	glm::vec3 TDTransform::GetForward()
	{
		return TransformDirection(glm::vec3(0, 0, 1), GetModel());
	}

	glm::vec3 TDTransform::GetUp()
	{
		return TransformDirection(glm::vec3(0, 1, 0), GetModel());
	}

	glm::vec3 TDTransform::GetRight()
	{
		return TransformDirection(glm::vec3(1, 0, 0), GetModel());
	}

	glm::vec3 TDTransform::TransfromToLocalSpace(glm::vec3 & direction)
	{
		return TransformDirection(direction, GetModel());
	}

	glm::vec3 TDTransform::TransformDirection(const  glm::vec3 & pDirection, const  glm::mat4 & pMatrix)const
	{
		return glm::normalize(glm::vec3(pMatrix * glm::vec4(pDirection, 0.0f)));
	}

	glm::quat TDTransform::GetQuatRot() const
	{
		CheckNAN(_qrot);
		return _qrot;
	}

	TDTransform * TDTransform::GetParent() const
	{
		return parent;
	}

	bool TDTransform::IsChanged()
	{
		if (oldpos != _pos)
			return true;

		if (oldqrot != _qrot)
			return true;

		if (oldscale != _scale)
			return true;

		return false;
	}
	void TDTransform::Update()
	{
		UpdateModel = true;
		GetModel();
	}

	glm::mat4 TDTransform::GetModel()
	{
		if (UpdateModel)
		{
			glm::mat4 posMat = glm::translate(_pos);
			glm::mat4 scaleMat = glm::scale(_scale);
			glm::mat4 rotMat = glm::toMat4(_qrot);
			CacheLocalModel = (posMat * rotMat * scaleMat);
			UpdateModel = false;
		}
		if (parent != nullptr)
		{
			parentMatrix = parent->GetModel();
			CacheModel = parentMatrix * CacheLocalModel;
		}
		else
		{
			//	parentMatrix = glm::mat4(1);
			CacheModel = CacheLocalModel;
		}

		_rot = glm::eulerAngles(_qrot);
		return CacheModel;
	}

	///------------------------------------------------------
	//setters	

	void TDTransform::SetPos(const glm::vec3 & pos)
	{
		UpdateModel = true;
		oldpos = _pos;
		_pos = pos;
		CheckNAN(_pos);
		GetModel();
	}

	void TDTransform::SetEulerRot(const glm::vec3 & rot)
	{
		UpdateModel = true;
		oldqrot = this->_qrot;
		this->_qrot = glm::quat(glm::radians(rot));
		CheckNAN(_qrot);
		GetModel();
	}

	void TDTransform::SetScale(const glm::vec3 & scale)
	{
		UpdateModel = true;
		this->oldscale = this->_scale;
		this->_scale = scale;
		CheckNAN(_scale);
		GetModel();
	}

	void TDTransform::AddRotation(glm::vec3 & rot)
	{
		UpdateModel = true;
		this->_rot += rot;
	}

	void TDTransform::RotateAboutAxis(glm::vec3 & axis, float amt)
	{
		UpdateModel = true;
		oldqrot = this->_qrot;
		this->_qrot *= glm::angleAxis((amt), glm::normalize(axis));
		CheckNAN(_qrot);
		GetModel();
	}

	void TDTransform::SetQrot(const glm::quat & val)
	{
		UpdateModel = true;
		_qrot = val;
		CheckNAN(_qrot);
		GetModel();
	}

	void TDTransform::SetParent(TDTransform * Parent)
	{
		UpdateModel = true;
		parent = Parent;
		GetModel();
	}

	void TDTransform::TranslatePos(const glm::vec3 & pos)
	{
		UpdateModel = true;
		_pos += pos;
		GetModel();
	}

	void TDTransform::MakeRotationFromXY(const glm::vec3 & Fwd, const glm::vec3 & up)
	{
		//_qrot = glm::)
	}

	glm::vec3 TDTransform::GetEulerRot() const
	{
		return _rot;
	}

	glm::vec3 TDTransform::GetPos()
	{
		//GetModel();
		return glm::vec3(CacheModel[3][0], CacheModel[3][1], CacheModel[3][2]);
	}

	glm::vec3 TDTransform::GetScale() const
	{
		return glm::vec3(CacheModel[0][0], CacheModel[1][1], CacheModel[2][2]);
	}
}