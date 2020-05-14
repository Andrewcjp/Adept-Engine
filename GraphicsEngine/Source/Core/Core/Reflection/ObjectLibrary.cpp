#include "ObjectLibrary.h"
ObjectLibrary* ObjectLibrary::Instance = nullptr;;

void ObjectLibrary::Register(uint64 TypeId, std::string classname, std::function<IReflect*()> Constructor)
{
	ClassType Type;
	Type.Constructor = Constructor;
	Type.Name = classname; 
	Type.TypeId = TypeId;
	Get()->m_TypeMap.emplace(TypeId, Type);
}

ObjectLibrary * ObjectLibrary::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ObjectLibrary();
	}
	return Instance;
}

std::vector<const ClassType*> ObjectLibrary::GetAllItemsOfType(uint64 Typeid)
{
	std::vector<const ClassType*> types;
	for (auto itor = Instance->m_TypeMap.begin(); itor != Instance->m_TypeMap.end(); itor++)
	{
		types.push_back(&itor->second);
	}
	return types;
}

