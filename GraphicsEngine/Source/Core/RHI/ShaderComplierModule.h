#pragma once
#include "ShaderBase.h"
#include "ShaderProgramBase.h"
#include "DeviceContext.h"
struct ShaderSourceFile;
class Shader;
struct ShaderByteCodeBlob
{
	void* ByteCode = nullptr;
	uint64 Length = 0;
	ShaderByteCodeBlob() {};
	ShaderByteCodeBlob(void* data, uint64 length)
	{
		ByteCode = data;
		Length = length;
	}
};

struct ShaderComplieItem
{
	ShaderSourceFile* Data = nullptr;
	EShaderError::Type Result = EShaderError::SHADER_ERROR_MAXCOUNT;
	std::string EntryPoint = "main";
	EShaderType::Type Stage = EShaderType::SHADER_UNDEFINED;
	std::vector<ShaderProgramBase::Shader_Define> Defines;
	std::string ShaderName = "";
	//shader model?
	ShaderByteCodeBlob* Blob = nullptr;
	ShaderByteCodeBlob* ReflectionBlob = nullptr;
	EShaderSupportModel::Type ShaderModel = EShaderSupportModel::SM6;
	EPlatforms::Type TargetPlatfrom = EPlatforms::Limit;
	bool CacheHit = false;
	void ResetOutput()
	{
		CacheHit = false;
		Blob = nullptr;
		ReflectionBlob = nullptr;
		Result = EShaderError::SHADER_ERROR_MAXCOUNT;
	}
	//compile options:
	bool ComplieShaderDebug = false;
};

class IShaderComplier : public IModuleInterface
{
public:
	virtual ~IShaderComplier()
	{}

	virtual void ComplieShader(ShaderComplieItem* shader) { shader->Result = EShaderError::SHADER_ERROR_CREATE; };
	virtual void InitWithData(void* Data) {};
	virtual bool SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderComplieItem* item) { return false; }
};