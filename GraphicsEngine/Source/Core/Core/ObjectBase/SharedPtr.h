#pragma once
#include "../IRefCount.h"

template <typename T>
class SharedPtr
{
private:
	IRefCount* m_Object{ nullptr };
	T* ObjectRaw = nullptr;
public:
	SharedPtr()
	{}
	//Constructor
	SharedPtr(T* object)
		: m_Object{ object }
	{
		if (object != nullptr)
		{
			m_Object->AddRef();
			ObjectRaw = object;
		}
	}
	//Destructor
	virtual ~SharedPtr()
	{
//		std::cout << "releasing SharedPtr! Ref count is " << m_Object->GetRefCount() - 1 << std::endl;
		if (ObjectRaw != nullptr)
		{
			float t = 0;
		}
		SafeRefRelease(ObjectRaw);
	}
	// Copy Constructor
	SharedPtr(const SharedPtr<T>& other)
		: m_Object{ other.m_Object }
	{
		if (m_Object != nullptr)
		{
			m_Object->AddRef();
		}		
		//std::cout << "Copied SharedPtr! Ref count is "
		//	<< m_Object->GetRefCount() << std::endl;
	}
	// Overloaded Assignment Operator
	SharedPtr<T>& operator=(const SharedPtr<T>& other)
	{
		if (this != &other)
		{
			if (m_Object)
			{
				SafeRefRelease(ObjectRaw);
			}
			m_Object = other.m_Object;
			ObjectRaw = other.ObjectRaw;
			m_Object->AddRef();
		}
	//	std::cout << "Assigning SharedPtr! Ref count is " << m_Object->GetRefCount() << std::endl;
		return *this;
	}
	SharedPtr<T>& operator=(T* object)
	{
		if (object != nullptr)
		{
			m_Object = object;
			m_Object->AddRef();
			ObjectRaw = object;
		}
		return *this;
	}
	//Dereference operator
	T& operator*()
	{
		return *m_Object;
	}
	//Member Access operator
	T* operator->()
	{
		return ObjectRaw;
	}
	T* Get() const 
	{
		return ObjectRaw;
	}

	bool IsValid()
	{
		return m_Object != nullptr;
	}

	bool operator== (const SharedPtr<T> &c1) const 
	{
		return m_Object == c1.m_Object;
	}

	bool operator!= (const SharedPtr<T> &c1) const
	{
		return m_Object != c1.m_Object;
	}
};

