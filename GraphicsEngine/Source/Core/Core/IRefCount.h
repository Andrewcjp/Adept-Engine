#pragma once
//macro That Removes ref and deletes the object if the count == 0; Also Nulls the pointer for safety
#define SafeRefRelease(target) SafeRHIRefRelease(target);
#define SafeRHIRefRelease(target) if(target != nullptr){if(target->ReleaseRef() == 0){EnqueueSafeRHIRelease(target); target = nullptr;}}
class IRefCount
{
public:
	void AddRef()
	{
		refcount++;
	};
	void DecrementRef()
	{
		RemoveRef();
	}
	int GetRefCount()
	{
		return refcount;
	};
	bool ReleaseRef()
	{
		if (GetRefCount() > 1)
		{
			DecrementRef();
		}
		else
		{
			if (GetRefCount() == 1)
			{
				DecrementRef();
				return false;
			}
		}
		return false;
	}
private:
	void RemoveRef()
	{
		refcount--;
	}
	int refcount = 0;
};