#pragma once

class GPUStateCache
{
public:
	static void Create()
	{
		if (instance == nullptr)
		{
			instance = new GPUStateCache();
		}
	}
	static void Destory()
	{
		if (instance != nullptr)
		{
			delete instance;
		}
	}
	~GPUStateCache();
	static GPUStateCache* instance;
	static bool CheckStateOfUnit(int unit, int state)
	{
		//return false;
		if (instance->TextureUnits[unit] == state)
		{
			return true;
		}
		return false;
	}
	static void UpdateUnitState(int unit, int state)
	{
		instance->TextureUnits[unit] = state;
	}
	static bool CheckCurrentUniformBuffer(int buffer)
	{
		return (instance->uniformBufferState == buffer);
	}
	static void UpdateCurrentUniformBuffer(int state)
	{
		instance->uniformBufferState = state;
	}
	class FrameBuffer* TextureBuffers[10] = { nullptr };
private:
	GPUStateCache();
	int TextureUnits[20];
	int uniformBufferState = 0;


};

