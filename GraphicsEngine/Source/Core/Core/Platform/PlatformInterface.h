#pragma once
class IShaderComplier;
class BasePlatformInterface
{
public:
	static void Init() {};
	static void RegisterShaderCompliers(std::vector<IShaderComplier*>& Shadercompliers) {};
};