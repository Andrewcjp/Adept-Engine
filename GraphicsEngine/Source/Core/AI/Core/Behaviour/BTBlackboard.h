#pragma once
namespace EBTBBValueType
{
	enum Type
	{
		Vector,
		Float,
		Integer,
		Limit
	};
};
struct BTValue
{
	EBTBBValueType::Type ValueType = EBTBBValueType::Limit;
	glm::vec3 vector = glm::vec3();
	int IntValue = 0;
	float FloatValue = 0.0f;
	template<class T>
	T* GetValue()
	{
		return (T*)GetValuePtr();
	}
private:
	void* GetValuePtr()
	{
		switch (ValueType)
		{
		case EBTBBValueType::Float:
			return &FloatValue;
			break;
		case EBTBBValueType::Integer:
			return &IntValue;
			break;
		case EBTBBValueType::Vector:
			return &vector;
			break;
		}
		return nullptr;
	}
};
class BTBlackboard
{
public:
	BTValue* AddValue(EBTBBValueType::Type type);
	BTBlackboard();
	~BTBlackboard();
	std::vector<BTValue*> values;
};

