#pragma once
#include "RHI/BaseTexture.h"
#include "Mesh/MeshPipelineController.h"
#include "Mesh/MaterialShader.h"
#include "Mesh/MaterialTypes.h"


class RHIBuffer;
class Shader_NodeGraph;
class Asset_Shader;
class MaterialShader;
class Material
{
public:

	void UpdateShaderData();
	CORE_API Material(Asset_Shader* shader);
	~Material();
	void SetMaterialActive(class RHICommandList * list, ERenderPass::Type Pass);
	//creates this material after all props have been setup
	void Init();
	void UpdateBind(std::string Name, BaseTextureRef NewTex);
	BaseTexture* GetTexturebind(std::string Name);
	typedef std::pair<std::string, TextureBindData> FlatMap;
	void SetDisplacementMap(BaseTexture* tex);
	CORE_API void SetNormalMap(BaseTexture * tex);
	CORE_API void SetDiffusetexture(BaseTextureRef tex);
	bool HasNormalMap();
	static Material * GetDefaultMaterial();
	void ProcessSerialArchive(class Archive* A);
	static constexpr const char* DefuseBindName = "DiffuseMap";
	RHIBuffer* MaterialDataBuffer = nullptr;
	EMaterialRenderType::Type GetRenderPassType();
	MaterialShaderComplieData MaterialCData;
	static Shader* GetDefaultMaterialShader();
	CORE_API static Material* CreateDefaultMaterialInstance();
	Shader_NodeGraph* GetShader();
	void SetFloat(std::string name, float value);
	bool IsComplied();
	void SetReceiveShadow(bool state);
private:
	TextureBindSet * CurrentBindSet = nullptr;
	void SetupDefaultBinding(TextureBindSet* TargetSet);
	MaterialShader* ShaderInterface = nullptr;
	ParmeterBindSet ParmbindSet;
};

