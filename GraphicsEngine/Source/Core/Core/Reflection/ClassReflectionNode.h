#pragma once

struct ClassReflectionData;
//Sits on a reflectional object defining the values used and provides setters and getter for them.
namespace MemberValueType
{
	enum Type
	{
		Int,
		Float,
		String,
		Vector2,
		Vector3,
		Vector4,
		Bool,
		Limit
	};
};


class ClassReflectionNode
{
public:
	ClassReflectionNode() {}
	ClassReflectionNode(std::string name, MemberValueType::Type type, void* Ptr) :
		m_MemberName(name), m_Type(type), m_pDataPtr(Ptr)
	{

	}
	std::string m_MemberName = "";
	std::string m_DisplayName = "";
	MemberValueType::Type m_Type = MemberValueType::Limit;
	void* m_pDataPtr = nullptr;

	std::string GetDisplayName()
	{
		if (m_DisplayName.length() > 0)
		{
			return m_DisplayName;
		}
		return m_MemberName;
	}
	float GetAsFloat()
	{
		ensure(m_Type == MemberValueType::Float);
		return *((float*)m_pDataPtr);
	}
	int GetAsInt()
	{
		ensure(m_Type == MemberValueType::Int);
		return *((int*)m_pDataPtr);
	}
	bool GetAsBool()
	{
		ensure(m_Type == MemberValueType::Bool);
		return *((bool*)m_pDataPtr);
	}
};
struct ClassReflectionData
{
	std::vector<ClassReflectionNode*> Data;
	void Add(ClassReflectionNode* node)
	{
		Data.push_back(node);
	}
	ClassReflectionNode* Last()
	{
		if (Data.size() == 0)
		{
			return nullptr;
		}
		return Data[Data.size() - 1];
	}
};
#define CLASS_BODY_Reflect() void InitReflection();
#define CLASS_BODY() void ProcessSerialArchive(Archive* A){}; CLASS_BODY_Reflect();
#define CALL_CONSTRUCTOR() InitReflection();