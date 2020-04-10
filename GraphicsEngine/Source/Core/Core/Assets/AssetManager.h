#pragma once

#include "Core/Utils/StringUtil.h"
#include "Core/Engine.h"
#include "AssetTypes.h"
class BaseTexture;
class IniHandler;
class BaseAsset;
class TextureHandle;
struct PlatformBuildSettings
{
	int MaxWidth = 1024;
	int MaxHeight = 1024;
	bool ClampTextures = true;
};
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
struct ShaderSourceFile
{
	std::string Source;
	std::vector<std::string> RootConstants;
	uint32 LineCount = 0;
	static const uint VersionNum = 1;
};
class AssetManager
{
public:


	static AssetManager* instance;
	CORE_API static AssetManager* Get();
	~AssetManager();
	static void StartAssetManager();
	static void ShutDownAssetManager();
	void LoadFromShaderDir();
	CORE_API bool LoadShaderMetaFile(std::string CSOpath, ShaderSourceFile ** file);
	CORE_API ShaderSourceFile* LoadFileWithInclude(std::string name);
	static void RegisterMeshAssetLoad(std::string name);
	bool ProcessTexture(TextureHandle* Handle);
	CORE_API static BaseTextureRef DirectLoadTextureAsset(std::string name, TextureImportSettings settigns = TextureImportSettings(), class DeviceContext * Device = nullptr);
	CORE_API static const std::string GetShaderPath();
	CORE_API static const std::string GetContentPath();
	CORE_API static const std::string GetDDCPath();
	CORE_API static const std::string GetScriptPath();
	CORE_API static const std::string GetRootDir();
	CORE_API static const std::string GetGeneratedDir();
	CORE_API static const std::string DirectGetGeneratedDir();
	CORE_API static const std::string GetSettingsDir();
	CORE_API static const std::string GetShaderCacheDir(EPlatforms::Type Platform = EPlatforms::Limit);
	const std::string GetDriverShaderCacheDir();
	BaseAsset * CreateOrGetAsset(std::string path);
	void TestAsset();
	static const PlatformBuildSettings& GetSettings();
	CORE_API static std::string GetPlatformDirName();
	CORE_API void WriteShaderMetaFile(ShaderSourceFile* file, std::string path, EPlatforms::Type platform = EPlatforms::Limit);
	static const std::string DDCName;
private:
	AssetManager();
	void Init();

	void InitAssetSettings(EPlatforms::Type Platform);

	std::map<std::string, ShaderSourceFile*> ShaderSourceMap;
	std::map<std::string, std::string> MeshFileMap;


	//Cached Paths
	void SetupPaths();
	std::string RootDir = "";
	std::string SettingsDir = "";
	std::string ContentDirPath = "";
	std::string ShaderDirPath = "";
	std::string DDCDirPath = "";
	std::string ScriptDirPath = "";
	std::string GeneratedDirPath = "";
	
	IniHandler* INISaver = nullptr;
	PlatformBuildSettings PlatformSettings;

};

