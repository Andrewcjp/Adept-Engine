#pragma once
#include "Asserts.h"
#include "Core\Platform\PlatformCore.h"
#include "Utils\VectorUtils.h"
#define DEBUG_HOLDSTACKS _DEBUG
//macro That Removes ref and deletes the object if the count == 0; Also Nulls the pointer for safety
#define SafeRefRelease(target) SafeRHIRefRelease(target);
#define SafeRHIRefRelease(target) if(target != nullptr){if(target->ReleaseRef() == 0){EnqueueSafeRHIRelease(target); target = nullptr;}}
class  IRefCount
{
public:
	void AddRef()
	{
#if DEBUG_HOLDSTACKS
		Traces.push_back(PlatformMisc::CaptureStack(1));
#endif
		refcount++;
	};
	void DecrementRef()
	{
		RemoveRef();
#if DEBUG_HOLDSTACKS
		if (Traces.size() > 0)
		{
			Traces.erase(Traces.end() - 1);
		}
#endif		
	}
	int GetRefCount()
	{
		return refcount;
	};
	bool ReleaseRef()
	{
		DecrementRef();
		if (GetRefCount() <= 0)
		{
			return false;
		}
		return true;
	}
#if DEBUG_HOLDSTACKS
	void PrintAllStacks()
	{
		for (StackTrace S : Traces)
		{
			S.PrintStack();
		}
	}
#endif
private:
	void RemoveRef()
	{
		refcount--;
	}
	int refcount = 0;
#if DEBUG_HOLDSTACKS
	std::vector<StackTrace> Traces;
#endif
};