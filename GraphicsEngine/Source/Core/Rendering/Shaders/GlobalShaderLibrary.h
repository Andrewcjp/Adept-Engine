#pragma once

class Shader_Pair;
class GlobalShaderLibrary
{
public:
	static void Init();
	static Shader_Pair* BuildTopLevelVXShader;
	static Shader_Pair* VolumeDownSample;
	static Shader_Pair* LightCullingShader;
	static Shader_Pair* TiledLightingApplyShader;
	static Shader_Pair* FixedVelocityShaders;
};

