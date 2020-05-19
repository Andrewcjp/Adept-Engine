#pragma once

struct ClassReflectionData;
class IReflect;
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
		AssetPtr,
		Limit
	};
};
typedef uint64 ClassTypeID;

class ClassReflectionNode
{
public:
	ClassReflectionNode() {}
	ClassReflectionNode(std::string name, MemberValueType::Type type, void* Ptr) :
		m_MemberName(name), m_Type(type), m_pDataPtr(Ptr)
	{

	}

	IReflect* Owner = nullptr;
	std::string m_MemberName = "";
	std::string m_DisplayName = "";
	MemberValueType::Type m_Type = MemberValueType::Limit;
	void* m_pDataPtr = nullptr;

	void MapPropToFunctions(std::function<void(void*)> GetFunc, std::function<void(void*)> SetFunc)
	{
		m_SetFunc = SetFunc;
		m_GetFunc = GetFunc;
		IsVirtualProp = true;
	}
	bool IsVirtualProp = false;
	std::function<void(void*)> m_GetFunc;
	std::function<void(void*)> m_SetFunc;
	std::string GetDisplayName()
	{
		if (m_DisplayName.length() > 0)
		{
			return m_DisplayName;
		}
		return m_MemberName;
	}
	float GetAsFloat();
	int GetAsInt();
	bool GetAsBool();
	std::string GetAsString();
	glm::vec3 GetAsFloat3();
	//setters
	void SetString(std::string value);
	void SetBool(bool value);
	void SetFloat(float value);
	void SetInt(int value);
	void SetFloat3(glm::vec3 value);
	void SetFunc();
	void GetFunc();
	uint64 GetSize()const;
	uint64 TemplateType = 0;
	template<class T>
	T* GetAsT()
	{
		//ensure(m_Type == MemberValueType::AssetPtr);		
		return (T*)m_pDataPtr;
	}
	template<class T>
	void SetAsT(T* value)
	{
		((T*)m_pDataPtr) = value;
		Notify();
	}
	void Notify();
private:
};
struct ClassReflectionData
{
	std::vector<ClassReflectionNode*> Data;
	IReflect* Owner = nullptr;
	void Add(ClassReflectionNode* node)
	{
		node->Owner = Owner;
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
#define CLASS_BODY_Reflect() void InitReflection(); static uint64 TYPEID;
#define CLASS_BODY() void ProcessSerialArchive(Archive* A){}; CLASS_BODY_Reflect();
#define CALL_CONSTRUCTOR() InitReflection();