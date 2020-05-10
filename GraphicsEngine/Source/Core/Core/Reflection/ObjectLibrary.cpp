#include "ObjectLibrary.h"
ObjectLibrary* ObjectLibrary::Instance = nullptr;;

void ObjectLibrary::Register(uint64 TypeId, std::function<IReflect*()> Constructor)
{
	Get()->ConstructorMap.emplace(TypeId, Constructor);
}

ObjectLibrary * ObjectLibrary::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ObjectLibrary();
	}
	return Instance;
}

