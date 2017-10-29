#pragma once
#include <GLEW\GL\glew.h>
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
	static bool CheckStateOfUnit(int unit, GLuint state)
	{
		//return false;
		if (instance->TextureUnits[unit] == state)
		{
			return true;
		}
		return false;
	}
	static void UpdateUnitState(int unit, GLuint state)
	{
		instance->TextureUnits[unit] = state;
	}
	static bool CheckCurrentUniformBuffer(GLuint buffer)
	{
		return (instance->uniformBufferState == buffer);
	}
	static void UpdateCurrentUniformBuffer( GLuint state)
	{
		instance->uniformBufferState = state;
	}
	
private:
	GPUStateCache();
	GLuint TextureUnits[20];
	GLuint uniformBufferState = 0;

};

