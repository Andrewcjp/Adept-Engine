#pragma once
//macro That Removes ref and deletes the object if the count == 0; Also Nulls the pointer for safety
//#define SafeRefRelease(target)if(target != nullptr && (target)->GetRefCount() > 1){(target)->ReleaseRef();}else{delete (target); (target) = nullptr;}
#define SafeRefRelease(target) SafeRHIRefRelease(target);
#define SafeRHIRefRelease(target)if(target != nullptr && (target)->GetRefCount() > 1){(target)->ReleaseRef();}else\
{if((target) != nullptr && (target)->GetRefCount() == 1){ (target)->ReleaseRef();EnqueueSafeRHIRelease(target);}}
class IRefCount
{
public:
	void AddRef()
	{
		refcount++;
	};
	void ReleaseRef()
	{
		RemoveRef();
	}
	int GetRefCount()
	{
		return refcount;
	};
private:
	void RemoveRef()
	{
		refcount--;
	}
	int refcount = 0;
};