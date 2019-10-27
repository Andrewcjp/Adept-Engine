#pragma once
namespace TypeUtils
{
	template<class T>
	T GetFromInt(int i)
	{
		return (T)glm::clamp(i, 0, (int)T::Limit);
	}
};
namespace HashUtils
{
	template <class T>
	inline void hash_combine(uint64 & s, const T & v)
	{
		std::hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}
}