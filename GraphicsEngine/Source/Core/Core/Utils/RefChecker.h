#pragma once
//Used to Detect Object leaks!

#define DETECT_MEMORY_LEAKS 1
#if DETECT_MEMORY_LEAKS
class RefCheckerContainer
{
public:
	CORE_API virtual int LogRefs();
	CORE_API static void LogAllRefCounters();
	CORE_API static void Add(RefCheckerContainer* a);
	static RefCheckerContainer* Instance;
private:
	std::vector<RefCheckerContainer*> Checkers;
};

template<class T>
class  RefChecker : public RefCheckerContainer
{
public:
	RefChecker()
	{
		RefCheckerContainer::Add(this);
	};

	T* AddRef(T* Ref)
	{
		Refs.push_back(Ref);
		return Ref;
	}
	T* RemoveRef(T* Ref)
	{
		for (int i = 0; i < Refs.size(); i++)
		{
			if (Refs[i] == Ref)
			{
				Refs.erase(Refs.begin() + i);
				return Ref;
			}
		}
		ensure(false);
		return Ref;
	}
	int LogRefs()
	{
		check(Refs.size() == 0);
		if (Refs.size() != 0)
		{
			Log::LogMessage("Objects Leaked!", Log::Error);
		}
		return (int)Refs.size();
	}

private:
	std::vector<T*> Refs;

};
#define CreateChecker(Type) static RefChecker<Type>* RefChecker##Type = new RefChecker<Type>();
#define AddCheckerRef(Type,Object) RefChecker##Type->AddRef(Object);
#define RemoveCheckerRef(Type,Object) RefChecker##Type->RemoveRef(Object);
#else
#define CreateChecker(Type) 
#define AddCheckerRef(Type,Object)
#define RemoveCheckerRef(Type,Object) 
#endif
