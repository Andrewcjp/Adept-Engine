#pragma once
#include "Core/Assets/AssetPtr.h"
#include "Core/Reflection/IReflect.h"
#define USEHASH 1
namespace ShaderPropertyType
{
	enum Type
	{
		Float,
		Float2,
		Float3,
		Float4,
		Bool,
		Int,
		Texture,
	};
}

class Asset_Shader;
class TextureAsset;
class TextureHandle;
struct EMaterialPassType
{
	enum Type
	{
		Forward,
		Deferred,
		Both,
		Limit
	};
};

struct EMaterialRenderType
{
	enum Type
	{
		Opaque,
		Transparent,
		Limit
	};
};
//this data is used by a material to ask for the correct shader from the MaterialShader is owns.
struct MaterialShaderCompileData
{
	EMaterialPassType::Type RenderPassUsage = EMaterialPassType::Forward;
	EMaterialRenderType::Type MaterialRenderType = EMaterialRenderType::Opaque;
	Asset_Shader* Shader = nullptr;
	//used to further customize shaders
	std::vector<std::string> ShaderKeyWords;
	bool operator==(const MaterialShaderCompileData other)const;
	bool operator<(const MaterialShaderCompileData &o)  const;
	std::string ToString();
	int ToHash();
};


struct TextureBindData
{
	SharedPtr<BaseTexture> TextureObj;
	int RootSigSlot = 0;
	int RegisterSlot = 0;
};

UCLASS()
struct MaterialShaderParameter : public IReflect
{
	CLASS_BODY_Reflect();
	MaterialShaderParameter();
	MaterialShaderParameter(const MaterialShaderParameter& other);
	PROPERTY();
	int PropType = 0;
	PROPERTY();
	size_t OffsetInBuffer = 0;
	PROPERTY();
	AssetPtr<TextureAsset> m_TextureAsset;

	TextureHandle* Handle = nullptr;
	size_t GetSize() const;
};

UCLASS()
struct ParmeterBindSet : public IReflect
{
	CLASS_BODY_Reflect();
	ParmeterBindSet();
	ParmeterBindSet(const ParmeterBindSet& a);
	std::map<std::string, MaterialShaderParameter> BindMap;
	void AddParameter(std::string name, ShaderPropertyType::Type tpye);
	void SetFloat(std::string name, float f);
	float GetFloat(std::string name);
	void SetTexture(std::string name, TextureAsset* asset);
	size_t GetSize();
	void AllocateMemeory();
	~ParmeterBindSet();
	void* GetDataPtr();
	void ProcessSerialArchive(Archive * A);
	void BindTextures(RHICommandList* list);
	void MakeActive();
	typedef unsigned char* BYTE;
	void SeralizeText(Archive* A) override;
private:	
	unsigned char* data = nullptr;
	size_t cachedSize = 0;


};

