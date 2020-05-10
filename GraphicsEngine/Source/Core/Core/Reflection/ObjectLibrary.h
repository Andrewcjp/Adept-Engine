#pragma once
class ObjectLibrary
{
public:
	static void Register(uint64 TypeId, std::function<IReflect*()> Constructor);
	template<class T>
	static T* CreateInstance()
	{
		return nullptr;
	}
	template<class T>
	static T* Create(uint64 TypeId)
	{
		auto Itor = Instance->ConstructorMap.find(TypeId);
		if (Itor != Instance->ConstructorMap.end())
		{
			return (T*)Itor->second();
		}
		return nullptr;
	}
	static ObjectLibrary* Get();
private:
	static ObjectLibrary* Instance;
	std::map<uint64, std::function<IReflect*()>> ConstructorMap;
};

