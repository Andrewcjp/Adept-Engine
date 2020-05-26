#pragma once
#include "RHI/BaseTexture.h"
#include "Mesh/MeshPipelineController.h"
#include "Mesh/MaterialShader.h"
#include "Mesh/MaterialTypes.h"


class RHIBuffer;
class Shader_NodeGraph;
class Asset_Shader;
class MaterialShader;
class RHIBufferGroup;
class TextureHandle;
class Material
{
public:

	void UpdateShaderData();
	Material() {}
	CORE_API Material(Asset_Shader* shader);
	void SetParmaters(const ParmeterBindSet& set);
	~Material();
	void SetMaterialActive(RHICommandList * RESTRICT list, const MeshPassRenderArgs& Pass);
	//creates this material after all props have been setup
	void Init();
	void UpdateBind(std::string Name, BaseTextureRef NewTex);
	BaseTexture* GetTexturebind(std::string Name);
	typedef std::pair<std::string, TextureBindData> FlatMap;
	void SetDisplacementMap(BaseTexture* tex);
	CORE_API void SetNormalMap(BaseTexture * tex);
	CORE_API void SetDiffusetexture(BaseTextureRef tex);
	void SetTexture(std::string name, TextureHandle* handle);
	void SetTextureAsset(std::string name, std::string);
	bool HasNormalMap();
	static Material * GetDefaultMaterial();
	void ProcessSerialArchive(class Archive* A);
	static constexpr const char* DefuseBindName = "DiffuseMap";
	RHIBufferGroup* MaterialDataBuffer = nullptr;
	EMaterialRenderType::Type GetRenderPassType();
	
	static Shader* GetDefaultMaterialShader();
	CORE_API static Material* CreateDefaultMaterialInstance();
	Shader_NodeGraph* GetShader();
	void SetFloat(std::string name, float value);
	bool IsComplied();
	void SetReceiveShadow(bool state);
	Shader * GetShaderInstance(EMaterialPassType::Type pass);
	void SetRenderType(EMaterialRenderType::Type t);
	static std::string ShadowShaderstring;
	int GetInstanceDataSize();
	bool IsValidForInstancing();
	void* GetDataPtr();
	Asset_Shader* GetShaderAsset();
	TextureHandle* TestHandle = nullptr;
	TextureHandle* GetTexture(int index);
	ParmeterBindSet ParmbindSet;
private:
	bool NeedsUpdate = false;
	MaterialShaderComplieData MaterialCData;

	MaterialShader* ShaderInterface = nullptr;
	bool CurrnetShadowState = false;
};

