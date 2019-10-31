#pragma once
class Math
{
public:
	Math();
	~Math();
	template<class T>
	static T Min(const T &A, const T &B)
	{
		return std::min(A, B);
	}
	template<class T>
	static T Max(const T &A, const T &B)
	{
		return std::max(A, B);
	}
	//template<class T>
	//static T ConvertBytes(const T& AMT,const T& TargetPow,const T& CurrentPow)
	//{

	//}
	template<class T>
	static T MBToBytes(const T& AMT)
	{
		T PowValue = (T)std::powl(1024, 2);
		return AMT * PowValue;
	}

	template<class T>
	static T UDelta(const T& a, const T& b)
	{
		return (a > b) ? (a - b) : (b - a);
	}
};

