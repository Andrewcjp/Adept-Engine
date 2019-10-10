#pragma once
#include <functional>

namespace VectorUtils
{
	template<class T>
	bool Contains(std::vector<T>& Vector,const T & value)
	{
		for (int i = 0; i < Vector.size(); i++)
		{
			if (Vector[i] == value)
			{
				return true;
			}
		}
		return false;
	}

	template<class T>
	bool Contains(std::vector<T>& Vector, T & value, int& index)
	{
		for (int i = 0; i < Vector.size(); i++)
		{
			if (Vector[i] == value)
			{
				index = i;
				return true;
			}
		}
		return false;
	}
	template<class T>
	bool Contains_F(std::vector<T>& Vector, T & value, std::function<bool(T, T)> comparefunc)
	{
		for (int i = 0; i < Vector.size(); i++)
		{
			if (comparefunc(Vector[i], value))
			{
				return true;
			}
		}
		return false;
	}
	template<class T, typename F>
	bool Contains(std::vector<T>& Vector, T & value, F comparefunc)
	{
		std::function<bool(T, T)> t = comparefunc;
		return Contains_F<T>(Vector, value, t);
	}



	template<class T>
	bool Remove(std::vector<T>& Vector, T & value)
	{
		for (int i = 0; i < Vector.size(); i++)
		{
			if (Vector[i] == value)
			{
				Vector.erase(Vector.begin() + i);
				return true;
			}
		}
		return false;
	}
	template<class T>
	bool AddUnique(std::vector<T>& Vector, T & value)
	{
		for (int i = 0; i < Vector.size(); i++)
		{
			if (Vector[i] == value)
			{
				return false;
			}
		}
		Vector.push_back(value);
		return true;
	}
};