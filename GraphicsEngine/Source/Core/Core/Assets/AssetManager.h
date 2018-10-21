#pragma once

#include "Core/Utils/StringUtil.h"
#include "Core/Engine.h"
#include "AssetTypes.h"
class BaseTexture;
struct TextureAsset
{
	int Nchannels;
	int Width;
	int Height;
	std::string name;
	size_t ByteSize;
	size_t NameSize;
	unsigned char* image;
};

struct ShaderAsset
{
	std::string Name;
};

class AssetManager
{
public:
	enum AssetType
	{
		Texture,
		Shader,
		Meshr
	};
	struct Asset
	{
		AssetType type;
		void* DataPtr;
	};

	static AssetManager* instance;
	CORE_API static AssetManager* Get();
	~AssetManager();
	static void StartAssetManager();
	void LoadFromShaderDir();
	
	bool GetTextureAsset(std::string path, TextureAsset & asset, bool ABSPath = false);
	CORE_API std::string LoadFileWithInclude(std::string name);

	bool GetShaderAsset(std::string path, ShaderAsset &asset);
	std::map<std::string, TextureAsset>* GetTextureMap() { return &TextureAssetsMap; }
	std::map<std::string, std::string>* GetMeshMap() { return &MeshFileMap; }

	static void RegisterMeshAssetLoad(std::string name);
	CORE_API static BaseTexture * DirectLoadTextureAsset(std::string name, bool DirectLoad = false, class DeviceContext * Device = nullptr);
	CORE_API static const std::string GetShaderPath();
	CORE_API static const std::string GetContentPath();
	CORE_API static const std::string GetDDCPath();
	CORE_API static const std::string GetScriptPath();
	CORE_API static const std::string GetTextureGenScript();
	CORE_API static const std::string GetRootDir();
	static const std::string GetGeneratedDir();
	static const std::string DirectGetGeneratedDir();
private:
	AssetManager();
	bool HasCookedData = false;

	std::map<std::string, std::string> ShaderSourceMap;
	std::map<std::string, TextureAsset> TextureAssetsMap;
	std::map<std::string, ShaderAsset> ShaderMap;
	std::map<std::string, std::string> MeshFileMap;

	bool PreLoadTextShaders = true;
	bool UseCookedShaders = false;
	bool CookShaders = true;
	bool UseCookedtextures = true;

	std::string TextureCooked = "asset/TextureCooked.bin";
	size_t LoadedAssetSize = 0;
	std::string FileSplit = "|FileStart|";

	//include Handler
	const char * includeText = "#include";
	int	 includeLength = 9;
	//Cached Paths
	void SetupPaths();
	std::string RootDir = "";
	std::string ContentDirPath = "";
	std::string ShaderDirPath = "";
	std::string DDCDirPath = "";
	std::string TextureGenScriptPath = "";
	std::string ScriptDirPath = "";
	std::string GeneratedDirPath = "";
	std::string LoadShaderIncludeFile(std::string name, int limit,std::string Relative = std::string());
	static const std::string DDCName;
	const int MaxIncludeTreeLength = 10;
};

