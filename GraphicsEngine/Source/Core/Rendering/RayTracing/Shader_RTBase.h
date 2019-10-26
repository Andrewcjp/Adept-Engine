#pragma once
#include "RHI/Shader.h"
#include "RHI/RHIRootSigniture.h"
//A base class that handles the different types of ray shaders
namespace ERTShaderType
{
	enum Type
	{
		Hit,
		Miss,
		Intersection,
		AnyHit,
		RayGen,
		Limit
	};
}

class Shader_RTBase :public Shader
{
public:
	Shader_RTBase(DeviceContext * C, std::string Name, ERTShaderType::Type Stage);
	~Shader_RTBase();
	RHI_API ERTShaderType::Type GetStage();
	void AddExport(std::string Symbol);
	RHI_API std::vector<std::string>& GetExports();
	RHI_API std::wstring GetFirstExportWide();
	RHIRootSigniture LocalRootSig;
	void InitRS();

	static bool IsShaderSupported(const ShaderComplieSettings& args);

protected:
	ERTShaderType::Type ShaderStage = ERTShaderType::Limit;
	std::vector<std::string> ExportedSymbols;
	std::wstring ExportZeroWide;
};

