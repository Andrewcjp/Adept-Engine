#pragma once

class Shader_Pair;
class GlobalShaderLibrary
{
public:
	static void Init();
	static Shader_Pair* BuildTopLevelVXShader;
	static Shader_Pair* VolumeDownSample;
	
};

