#pragma once

#include "Core/Utils/StringUtil.h"
#include "Core/Engine.h"
#include "AssetTypes.h"
class BaseTexture;
class IniHandler;
class BaseAsset;
namespace ECompressionSetting
{
	enum Type
	{
		None,
		BC1,
		FP16,
		BRGA,
		Limit
	};
}
struct TextureImportSettings
{
	TextureImportSettings()
	{}
	TextureImportSettings(bool Forceload)
	{
		DirectLoad = Forceload;
	}
	ECompressionSetting::Type Compression = ECompressionSetting::BRGA;
	bool DirectLoad = false;
	std::string GetTypeString();
	int ForceMipCount = -1;
	bool InitOnAllDevices = true;
	bool IsCubeMap = false;
};
class AssetManager
{
public:


	static AssetManager* instance;
	CORE_API static AssetManager* Get();
	~AssetManager();
	static void StartAssetManager();
	void LoadFromShaderDir();
	CORE_API std::string LoadFileWithInclude(std::string name);
	static void RegisterMeshAssetLoad(std::string name);
	CORE_API static BaseTexture * DirectLoadTextureAsset(std::string name, TextureImportSettings settigns = TextureImportSettings(), class DeviceContext * Device = nullptr);
	CORE_API static const std::string GetShaderPath();
	CORE_API static const std::string GetContentPath();
	CORE_API static const std::string GetDDCPath();
	CORE_API static const std::string GetScriptPath();
	CORE_API static const std::string GetRootDir();
	CORE_API static const std::string GetGeneratedDir();
	CORE_API static const std::string DirectGetGeneratedDir();
	CORE_API static const std::string GetSettingsDir();
	BaseAsset * CreateOrGetAsset(std::string path);
	void TestAsset();
private:
	AssetManager();
	void Init();

	std::map<std::string, std::string> ShaderSourceMap;
	std::map<std::string, std::string> MeshFileMap;

	//include Handler
	const char * includeText = "#include";
	int	 includeLength = 9;
	//Cached Paths
	void SetupPaths();
	std::string RootDir = "";
	std::string SettingsDir= "";
	std::string ContentDirPath = "";
	std::string ShaderDirPath = "";
	std::string DDCDirPath = "";
	std::string ScriptDirPath = "";
	std::string GeneratedDirPath = "";
	std::string LoadShaderIncludeFile(std::string name, int limit, std::string Relative = std::string());
	static const std::string DDCName;
	const int MaxIncludeTreeLength = 10;
	IniHandler* INISaver = nullptr;
};

