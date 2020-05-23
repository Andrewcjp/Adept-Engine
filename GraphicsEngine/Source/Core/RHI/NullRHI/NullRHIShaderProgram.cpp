
#include "NullRHIShaderProgram.h"
#include "RHI/ShaderComplierModule.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Core/ShaderCache.h"
#include "Core/Assets/ShaderComplier.h"

NullRHIShaderProgram::NullRHIShaderProgram(DeviceContext * dev) :ShaderProgramBase(dev)
{}

EShaderError::Type NullRHIShaderProgram::AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint)
{
	ShaderComplieItem*  item = new ShaderComplieItem();
	item->ShaderName = filename;
	item->Defines = Defines;
	item->Data = AssetManager::Get()->LoadFileWithInclude(item->ShaderName + ".hlsl");
	item->Stage = type;
	item->ShaderModel = ShaderComplier::Get()->m_Config.ShaderModelTarget;
	item->EntryPoint = Entrypoint;
	ShaderByteCodeBlob* blob = ShaderCache::GetShader(item);
	if (blob == nullptr)
	{
		return EShaderError::SHADER_ERROR_COMPILE;
	}
	return EShaderError::SHADER_ERROR_NONE;
}

glm::ivec3 NullRHIShaderProgram::GetComputeThreadSize() const
{
	return glm::ivec3(1, 1, 1);
}
