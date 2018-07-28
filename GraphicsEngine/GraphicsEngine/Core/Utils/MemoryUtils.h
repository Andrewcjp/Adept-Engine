#pragma once
#include <vector>
namespace MemoryUtils
{
	template<typename T>
	static void DeleteVector(std::vector<T*>& target)
	{
		for (int i = 0; i < target.size(); i++)
		{
			delete target[i];
			target[i] = nullptr;
		}
		target.clear();
	}
	template<typename T>
	static void DeleteCArray(T* target, int Length)
	{
		for (int i = 0; i < Length; i++)
		{
			delete target[i];
			target[i] = nullptr;
		}
	}

};
