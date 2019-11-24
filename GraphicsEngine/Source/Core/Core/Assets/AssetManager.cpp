#include "AssetManager.h"
#include "Asset types/BaseAsset.h"
#include "Core/Utils/FileUtils.h"
#include "ImageIO.h"
#include "IniHandler.h"
#include "RHI/ShaderPreProcessor.h"
#ifdef PLATFORM_WINDOWS
#include <filesystem>
#endif
#include "Packaging/Cooker.h"
#include "Archive.h"

const std::string AssetManager::DDCName = "DerivedDataCache";
void AssetManager::LoadFromShaderDir()
{
	std::string path = GetShaderPath();
#ifdef PLATFORM_WINDOWS
	for (auto & p : std::experimental::filesystem::directory_iterator(path))
	{
		LoadFileWithInclude(p.path().filename().string());
	}
#endif
}

//and wrap the code to recreate when requested
//linked list of nodes written to disk binary
//each node stores string of id and size of data.
AssetManager* AssetManager::instance = nullptr;
void AssetManager::StartAssetManager()
{
	if (instance == nullptr)
	{
		instance = new AssetManager();
		instance->Init();
	}
}

void AssetManager::ShutDownAssetManager()
{
	SafeDelete(instance);
}

const std::string AssetManager::GetContentPath()
{
	if (instance != nullptr)
	{
		return instance->ContentDirPath;
	}
	return "";
}

const std::string AssetManager::GetShaderPath()
{
	if (instance != nullptr)
	{
		return instance->ShaderDirPath;
	}
	return "";
}

const std::string AssetManager::GetDDCPath()
{
	if (instance != nullptr)
	{
		return instance->DDCDirPath;
	}
	return "";
}

const std::string AssetManager::GetScriptPath()
{
	if (instance != nullptr)
	{
		return instance->ScriptDirPath;
	}
	return "";
}

const std::string AssetManager::GetRootDir()
{
	if (instance != nullptr)
	{
		return instance->RootDir;
	}
	return "";
}

const std::string AssetManager::GetGeneratedDir()
{
	if (instance != nullptr)
	{
		return instance->GeneratedDirPath;
	}
	return "";
}

const std::string AssetManager::DirectGetGeneratedDir()
{
	std::string GeneratedDirPath = Engine::GetExecutionDir() + "\\Saved\\";
	PlatformApplication::TryCreateDirectory(GeneratedDirPath);
	return GeneratedDirPath;
}

const std::string AssetManager::GetSettingsDir()
{
	return instance->SettingsDir;
}



BaseAsset* AssetManager::CreateOrGetAsset(std::string path)
{
	BaseAsset* Asset = nullptr;
	if (FileUtils::File_ExistsTest(BaseAsset::GetMetaFileName(path)))
	{
		Asset = new BaseAsset();
		Asset->LoadAsset(path);
	}
	else
	{
		//#Asset create the correct derived class here.
		Asset = new BaseAsset();
		Asset->GenerateNewAsset(path);
		Asset->SaveAsset();
	}
	return Asset;
}

void AssetManager::TestAsset()
{
	std::string TestTarget = GetContentPath() + "model test.mtl";
	BaseAsset* CreationTest = CreateOrGetAsset(TestTarget);
	ensure(CreationTest);
}

const PlatformBuildSettings & AssetManager::GetSettings()
{
	return Get()->PlatformSettings;
}

std::string AssetManager::GetPlatformDirName()
{
	if (Engine::GetIsCooking())
	{
		return EPlatforms::ToString(Engine::GetCookContext()->GetTargetPlatform());
	}
	return  EPlatforms::ToString(PlatformApplication::GetPlatform());
}

void AssetManager::WriteShaderMetaFile(ShaderSourceFile * file, std::string path)
{
	if (file->RootConstants.size() == 0)
	{		
		return;
	}
	std::string Metapath = GetShaderCacheDir() + path + ".meta";
	Archive* File = new Archive(Metapath, true);
	File->HandleArchiveBody("ShaderData");
	File->LinkStringArray(file->RootConstants, "RootConstants");
	File->EndHeaderWrite("ShaderData");
	File->Write();
}

bool AssetManager::LoadShaderMetaFile(std::string CSOpath, ShaderSourceFile** file)
{
	const std::string Metapath = GetShaderCacheDir() + CSOpath + ".meta";
	if (!FileUtils::File_ExistsTest(Metapath))
	{
		return false;
	}
	ShaderSourceFile* MetaData = new ShaderSourceFile();
	*file = MetaData;
	Archive* File = new Archive(Metapath);
	File->HandleArchiveBody("ShaderData");
	File->LinkStringArray(MetaData->RootConstants, "RootConstants");
	SafeDelete(File);
	return true;
}

const std::string AssetManager::GetShaderCacheDir()
{
	return AssetManager::GetDDCPath() + "Shaders\\" + GetPlatformDirName() + "\\";
}

void AssetManager::SetupPaths()
{
	RootDir = Engine::GetExecutionDir();
#if !BUILD_PACKAGE
	StringUtils::RemoveChar(RootDir, "\\Binaries");
	StringUtils::RemoveChar(RootDir, "\\binaries");
#endif
	ContentDirPath = RootDir + "\\Content\\";
	if (!FileUtils::File_ExistsTest(ContentDirPath))
	{
		PlatformApplication::DisplayMessageBox("Error", "No Content Dir");
		Engine::RequestExit(-1);
	}
	DDCDirPath = RootDir + "\\" + DDCName + "\\";
	PlatformApplication::TryCreateDirectory(DDCDirPath);

	GeneratedDirPath = Engine::GetExecutionDir() + "\\Saved\\";
	PlatformApplication::TryCreateDirectory(GeneratedDirPath);
#if !BUILD_PACKAGE
	ShaderDirPath = RootDir + "\\Shaders\\";
	if (!FileUtils::File_ExistsTest(ShaderDirPath))
	{
		PlatformApplication::DisplayMessageBox("Error", "No Shader Dir");
		Engine::RequestExit(-1);
	}
	ScriptDirPath = RootDir + "\\Scripts\\";
#endif
	SettingsDir = Engine::GetExecutionDir() + "\\Config";
	PlatformApplication::TryCreateDirectory(SettingsDir);
}

AssetManager::AssetManager()
{
	SetupPaths();
	PlatformApplication::TryCreateDirectory(GetDDCPath());
}

void AssetManager::Init()
{
	INISaver = new IniHandler();
	INISaver->LoadMainCFG();
	INISaver->SaveAllConfigProps();

	TestAsset();
	if (Engine::GetIsCooking())
	{
		InitAssetSettings(Engine::GetCookContext()->GetTargetPlatform());
	}
	else
	{
		InitAssetSettings(PlatformApplication::GetPlatform());
	}
}

void AssetManager::InitAssetSettings(EPlatforms::Type Platform)
{
	switch (Platform)
	{
		case EPlatforms::Windows:
		case EPlatforms::Linux:
			PlatformSettings.ClampTextures = false;
			PlatformSettings.MaxHeight = 8192;
			PlatformSettings.MaxWidth = 8192;
			break;
		case EPlatforms::Android:
			PlatformSettings.MaxHeight = 128;
			PlatformSettings.MaxWidth = 128;
			break;
	}
}

AssetManager * AssetManager::Get()
{
	if (instance == nullptr)
	{
		instance = new AssetManager();
	}
	return instance;
}

AssetManager::~AssetManager()
{}

ShaderSourceFile* AssetManager::LoadFileWithInclude(std::string name)
{
	ShaderSourceFile*  output;
	if (ShaderSourceMap.find(name) == ShaderSourceMap.end())
	{
		output = new ShaderSourceFile();
		output->Source = ShaderPreProcessor::LoadShaderIncludeFile(name, 0);
		ShaderPreProcessor::FindRootConstants(output);
		ShaderSourceMap.emplace(name, output);
	}
	else
	{
		output = ShaderSourceMap.at(name);
	}
	return output;
}

void AssetManager::RegisterMeshAssetLoad(std::string name)
{
	if (instance)
	{
		if (instance->MeshFileMap.find(name) == instance->MeshFileMap.end())
		{
			instance->MeshFileMap.emplace(name, name);
		}
	}
}

//#Files: Check time stamps!
BaseTextureRef AssetManager::DirectLoadTextureAsset(std::string name, TextureImportSettings settings, DeviceContext* Device)
{
	AssetPathRef Fileref = AssetPathRef(name);
#if WITH_EDITOR
	if (!FileUtils::File_ExistsTest(Fileref.GetFullPathToAsset()))
	{
		Log::OutS << "File '" << Fileref.Name << "' Does not exist" << Log::OutS;
		return nullptr;
	}
#endif
	RHITextureDesc Desc;
	Desc.IsCubeMap = settings.IsCubeMap;

	if (Fileref.GetFileType() == AssetFileType::DDS)
	{
#if WITH_EDITOR
		//Temp copy to DDC
		ensure(PlatformApplication::CopyFileToTarget(Fileref.GetFullPathToAsset(), GetDDCPath() + Fileref.GetBaseNameExtention()));
#endif
		Fileref.IsDDC = true;
	}

	//#Files: Deal with TGA to DDS 
	if (/*Fileref.GetFileType() == AssetFileType::DDS ||*/ StringUtils::Contains(name, ".tga") || settings.DirectLoad)
	{
		return RHI::CreateTexture(Fileref, Device, Desc);
	}

	//check the DDC for A Generated one
	std::string DDCRelFilepath = "\\" + DDCName + "\\" + Fileref.BaseName + ".DDS";
	Fileref.DDCPath = DDCRelFilepath;
	if (FileUtils::File_ExistsTest(GetRootDir() + DDCRelFilepath) && !PlatformApplication::CheckFileSrcNewer(GetRootDir() + DDCRelFilepath, Fileref.GetFullPathToAsset()) || Fileref.IsDDC)
	{
		Fileref.IsDDC = true;
		return RHI::CreateTexture(Fileref, Device, Desc);
}
	else
	{
		//File is not a DDS
#if BUILD_PACKAGE
		std::string Message = "File '" + Fileref.Name + "' Is missing from the cooked data";
		Log::LogMessage(Message, Log::Severity::Error);
		PlatformApplication::DisplayMessageBox("Error", Message);
#else
		Log::OutS << "File '" << Fileref.Name << "' Does not exist in the DDC Generating Now" << Log::OutS;
		//generate one! BC1_UNORM  
		std::string Args = " ";
		if (settings.ForceMipCount == 1)
		{
			Args.append(" -m 1 ");
		}
		else
		{
			Args.append("-pow2");
		}
		if (GetSettings().ClampTextures)
		{
			Args.append(" -w " + std::to_string(GetSettings().MaxWidth));
			Args.append(" -h " + std::to_string(GetSettings().MaxHeight));
		}
		Args.append(" -y ");
		Args.append(" -f " + settings.GetTypeString());
		Args.append('"' + Fileref.GetFullPathToAsset() + '"' + " ");
		//Log::LogMessage("Started Texconv.exe with: " + Args);
		PlatformApplication::ExecuteHostScript(GetScriptPath() + "Texconv.exe", Args, GetDDCPath(), false);
		if (FileUtils::File_ExistsTest(GetRootDir() + DDCRelFilepath))
		{
			Fileref.IsDDC = true;
			return RHI::CreateTexture(Fileref, Device, Desc);
		}
		else
		{
			Log::OutS << "File '" << Fileref.Name << "' Failed To Generate!" << Log::OutS;
		}
#endif
	}
	return ImageIO::GetDefaultTexture();
	}

std::string TextureImportSettings::GetTypeString()
{
	switch (Compression)
	{
		case ECompressionSetting::None:
			return " FP32 ";
			break;
		case ECompressionSetting::FP16:
			return " FP16 ";
		case ECompressionSetting::BRGA:
			return " BGRA ";
		case ECompressionSetting::BC1:
			return " BC1_UNORM ";
			break;
		case ECompressionSetting::Limit:
			break;
	}
	return "  ";
}
