#pragma once

class Shader_Pair;
class UAVFormatConverter
{
public:
	UAVFormatConverter();
	~UAVFormatConverter();

	static void UnPackToTmpResource(RHITexture** RefTarget, RHICommandList* list, RHITexture* Target);

	static void PackBacktoResource(RHITexture* PackedTarget, RHICommandList* list, RHITexture* Target);

	static UAVFormatConverter* Get();
	static UAVFormatConverter* instance;

private:
	Shader_Pair* UnPackShader = nullptr;
	Shader_Pair* PackShader = nullptr;
};

