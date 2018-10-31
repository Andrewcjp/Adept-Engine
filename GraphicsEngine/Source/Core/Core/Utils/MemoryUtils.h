#pragma once

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

	template<typename T>
	static void DeleteReleaseableCArray(T* target, int Length)
	{
		for (int i = 0; i < Length; i++)
		{
			if (target[i] != nullptr)
			{
				target[i]->Release();
				target[i] = nullptr;
			}
		}
	}
	template<typename T>
	static void DeleteReleaseableVector(std::vector<T*>& target)
	{
		for (int i = 0; i < target.size(); i++)
		{
			target[i]->Release();
			target[i] = nullptr;
		}
		target.clear();
	}
	template<typename S, typename T>
	static void DeleteReleaseableMap(std::map<S, T>& target)
	{
		std::map<S, T>::iterator it;
		for (it = target.begin(); it != target.end(); it++)
		{
			it->second.Release();
		}
	}
	namespace RHIUtil
	{
		template<typename T>
		static void DeleteRHICArray(T* target, int Length)
		{
			for (int i = 0; i < Length; i++)
			{
				EnqueueSafeRHIRelease(target[i]);
				target[i] = nullptr;
			}
		}
		template<typename T>
		static void DeleteVector(std::vector<T*>& target)
		{
			for (int i = 0; i < target.size(); i++)
			{
				EnqueueSafeRHIRelease(target[i]);
				target[i] = nullptr;
			}
			target.clear();
		}
	}
};
