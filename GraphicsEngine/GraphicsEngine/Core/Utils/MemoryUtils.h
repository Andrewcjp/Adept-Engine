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
		}
		target.clear();
	}
};

