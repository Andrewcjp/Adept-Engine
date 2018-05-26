#pragma once
//macro That Removes ref and deletes the object if the count == 0; Also Nulls the pointer for safety
#define SafeRelease(target)if((target)->GetRefCount() > 1){(target)->Release();}else{delete (target); (target) = nullptr;}
class IRefCount
{
public:
	void AddRef() { refcount++; };
	void Release()
	{
		RemoveRef();
	}	
	int GetRefCount()
	{
		return refcount;
	};
private:
	void RemoveRef() { refcount--; };
	int refcount = 0;
};