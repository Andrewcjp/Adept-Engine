#include "ClassReflectionNode.h"

float ClassReflectionNode::GetAsFloat()
{
	ensure(m_Type == MemberValueType::Float);
	GetFunc();
	return *((float*)m_pDataPtr);
}

int ClassReflectionNode::GetAsInt()
{
	ensure(m_Type == MemberValueType::Int);
	return *((int*)m_pDataPtr);
}

bool ClassReflectionNode::GetAsBool()
{
	ensure(m_Type == MemberValueType::Bool);
	return *((bool*)m_pDataPtr);
}

std::string ClassReflectionNode::GetAsString()
{
	ensure(m_Type == MemberValueType::String);
	GetFunc();
	return *((std::string*)m_pDataPtr);
}

glm::vec3 ClassReflectionNode::GetAsFloat3()
{
	ensure(m_Type == MemberValueType::Vector3);
	GetFunc();
	return *((glm::vec3 *)m_pDataPtr);
}

//setters

void ClassReflectionNode::SetString(std::string value)
{
	ensure(m_Type == MemberValueType::String);
	*((std::string*)m_pDataPtr) = value;
}

void ClassReflectionNode::SetBool(bool value)
{
	ensure(m_Type == MemberValueType::Bool);
	*((bool*)m_pDataPtr) = value;
}

void ClassReflectionNode::SetFloat(float value)
{
	ensure(m_Type == MemberValueType::Float);
	*((float*)m_pDataPtr) = value;
}

void ClassReflectionNode::SetInt(int value)
{
	ensure(m_Type == MemberValueType::Int);
	*((int*)m_pDataPtr) = value;
}

void ClassReflectionNode::SetFloat3(glm::vec3 value)
{
	ensure(m_Type == MemberValueType::Vector3);
	*((glm::vec3*)m_pDataPtr) = value;
	SetFunc();
}

void ClassReflectionNode::SetFunc()
{
	if (IsVirtualProp && m_SetFunc != nullptr)
	{
		m_SetFunc(m_pDataPtr);
	}
}

void ClassReflectionNode::GetFunc()
{
	if (IsVirtualProp && m_GetFunc != nullptr)
	{
		m_GetFunc(m_pDataPtr);
	}
}

uint64 ClassReflectionNode::GetSize() const
{
	switch (m_Type)
	{
	case MemberValueType::Bool:
	case MemberValueType::Int:
		return sizeof(int);
		break;
	case MemberValueType::Float:
		return sizeof(float);
		break;
	case MemberValueType::String:
		return sizeof(((std::string*)m_pDataPtr)->c_str());
		break;
	case MemberValueType::Vector2:
		return sizeof(glm::vec2);
		break;
	case MemberValueType::Vector3:
		return sizeof(glm::vec3);
		break;
	case MemberValueType::Vector4:
		return sizeof(glm::vec4);
	case MemberValueType::Limit:
	default:
		break;
	}
	return 0;
}
