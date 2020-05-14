#pragma once
struct ClassType
{
	std::string Name = "";
	std::function<IReflect*()> Constructor;
	uint64 TypeId = 0;
};
class ObjectLibrary
{
public:
	static void Register(uint64 TypeId, std::string classname, std::function<IReflect*()> Constructor);
	template<class T>
	static T* CreateInstance()
	{
		return nullptr;
	}
	template<class T>
	static T* Create(uint64 TypeId)
	{
		auto Itor = Instance->m_TypeMap.find(TypeId);
		if (Itor != Instance->m_TypeMap.end())
		{
			return (T*)Itor->second.Constructor();
		}
		return nullptr;
	}
	static ObjectLibrary* Get();

	static std::vector<const ClassType*> GetAllItemsOfType(uint64 Typeid);
private:
	static ObjectLibrary* Instance;
	std::map<uint64, ClassType> m_TypeMap;
};

