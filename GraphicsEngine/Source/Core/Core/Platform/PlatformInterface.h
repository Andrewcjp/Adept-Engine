#pragma once
class IShaderCompiler;
class BasePlatformInterface
{
public:
	static void Init() {};
	static void RegisterShaderCompilers(std::vector<IShaderCompiler*>& ShaderCompilers) {};
};