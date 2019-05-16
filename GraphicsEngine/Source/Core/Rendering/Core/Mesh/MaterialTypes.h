#pragma once

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
	};
}

class Asset_Shader;
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
struct MaterialShaderComplieData
{
	EMaterialPassType::Type RenderPassUsage = EMaterialPassType::Forward;
	EMaterialRenderType::Type MaterialRenderType = EMaterialRenderType::Opaque;
	Asset_Shader* Shader = nullptr;
	//used to further customize shaders
	std::vector<std::string> ShaderKeyWords;
	bool operator==(const MaterialShaderComplieData other)const;
	bool operator<(const MaterialShaderComplieData &o)  const;
	std::string ToString();
};


struct TextureBindData
{
	SharedPtr<BaseTexture> TextureObj;
	int RootSigSlot = 0;
	int RegisterSlot = 0;
};

struct TextureBindSet
{
	std::map<std::string, TextureBindData> BindMap;
	void AddBind(std::string name, int index, int Register)
	{
		BindMap.emplace(name, TextureBindData{ nullptr, index ,Register });
	}
};

struct MaterialShaderParameter
{
	ShaderPropertyType::Type PropType = ShaderPropertyType::Float;
	size_t GetSize() const;
	size_t OffsetInBuffer = 0;
};

struct ParmeterBindSet
{
	std::map<std::string, MaterialShaderParameter> BindMap;
	void AddParameter(std::string name, ShaderPropertyType::Type tpye);
	void SetFloat(std::string name, float f);
	size_t GetSize();
	void AllocateMemeory();
	~ParmeterBindSet();
	void* GetDataPtr();
private:
	unsigned char* data = nullptr;
};

