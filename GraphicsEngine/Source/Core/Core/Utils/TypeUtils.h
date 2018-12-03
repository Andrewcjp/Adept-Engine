#pragma once
namespace TypeUtils
{
	template<class T>
	T GetFromInt(int i)
	{
		return (T)glm::clamp(i, 0, (int)T::Limit);
	}
};
