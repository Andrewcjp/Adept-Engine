#include "stdafx.h"
#include "AssetManager.h"
#include "RHI/Shader.h"
#include "RHI/ShaderProgramBase.h"
#include <iostream>
#include <ios>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string>
#include <iostream>
#include <filesystem>
#include <experimental/filesystem>
#include <iomanip>
#include "Core/Engine.h"
#include <SOIL.h>
#include "Core/Utils/StringUtil.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/FileUtils.h"
#include "RHI/DeviceContext.h"
#include "Core/Platform/PlatformCore.h"
#include "ImageIO.h"
const std::string AssetManager::DDCName = "DerivedDataCache";
void AssetManager::LoadFromShaderDir()
{
	std::string path = GetShaderPath();
	for (auto & p : std::experimental::filesystem::directory_iterator(path))
	{
		LoadFileWithInclude(p.path().filename().string());
	}
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
	}
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

const std::string AssetManager::GetTextureGenScript()
{
	if (instance != nullptr)
	{
		return instance->TextureGenScriptPath;
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
	std::string GeneratedDirPath = Engine::GetExecutionDir() + "\\Saved";
	PlatformApplication::TryCreateDirectory(GeneratedDirPath);
	return GeneratedDirPath;
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
		Engine::Exit(-1);
	}
	DDCDirPath = RootDir + "\\" + DDCName + "\\";
	PlatformApplication::TryCreateDirectory(DDCDirPath);

	GeneratedDirPath = Engine::GetExecutionDir()+"\\Saved"; 
	PlatformApplication::TryCreateDirectory(GeneratedDirPath);
#if !BUILD_PACKAGE
	ShaderDirPath = RootDir + "\\Shaders\\";
	if (!FileUtils::File_ExistsTest(ShaderDirPath))
	{
		PlatformApplication::DisplayMessageBox("Error", "No Shader Dir");
		Engine::Exit(-1);
	}
	TextureGenScriptPath = RootDir + "\\Scripts\\ConvertToDDS.bat";
	ScriptDirPath = RootDir + "\\Scripts\\";
	if (!FileUtils::File_ExistsTest(TextureGenScriptPath))
	{
		PlatformApplication::DisplayMessageBox("Error", "Texture Get Script Missing");
	}
#endif
}

AssetManager::AssetManager()
{
	SetupPaths();
	PlatformApplication::TryCreateDirectory(GetDDCPath());
#if !BUILD_SHIPPING
	LoadFromShaderDir();
#endif
#if 0//BUILD_PACKAGE
	LoadTexturesFromDir();
#else
	//LoadCookedShaders();
#endif
	//Log::OutS  << "Shaders Loaded in " << ((PerfManager::get_nanos() - StartTime) / 1e6f) << "ms " << Log::OutS;
	//Log::OutS  << "Texture Asset Memory " << (float)LoadedAssetSize / 1e6f << "mb " << Log::OutS;
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

bool AssetManager::GetTextureAsset(std::string path, TextureAsset &asset, bool ABSPath)
{
	if (HasCookedData)
	{

	}
	else
	{
		if (TextureAssetsMap.find(path) == TextureAssetsMap.end())
		{
			TextureAsset newasset;
			unsigned char* image = nullptr;
			std::string fullpath = "";
			if (ABSPath)
			{
				fullpath = path;
			}
			else
			{
				fullpath = Engine::GetExecutionDir();
				fullpath.append(path);
			}
			if (fullpath.find(".tga") != -1)
			{
				int bpp = 0;
				ImageIO::LoadTGA(fullpath.c_str(), &image, &newasset.Width, &newasset.Height, &bpp, &newasset.Nchannels);
			}
			else
			{
				image = SOIL_load_image(fullpath.c_str(), &newasset.Width, &newasset.Height, &newasset.Nchannels, SOIL_LOAD_RGBA);
			}
			if (image == nullptr)
			{
				Log::OutS << "Load texture Error " << fullpath << Log::OutS;
				return false;
			}
			newasset.image = image;
			newasset.ByteSize = (newasset.Width* newasset.Height) *(newasset.Nchannels * sizeof(unsigned char));
			LoadedAssetSize += newasset.ByteSize;
			StringUtils::RemoveChar(fullpath, Engine::GetExecutionDir());
			newasset.name = fullpath;
			newasset.NameSize = fullpath.length();
			TextureAssetsMap.emplace(path, newasset);
			asset = TextureAssetsMap.at(path);
		}
		else
		{
			asset = TextureAssetsMap.at(path);
			return true;
		}
		return true;
	}
	return false;
}

std::string AssetManager::LoadFileWithInclude(std::string name)
{
	std::string  output;
	if (ShaderSourceMap.find(name) == ShaderSourceMap.end())
	{
		output = LoadShaderIncludeFile(name, 0);
		ShaderSourceMap.emplace(name, output);
	}
	else
	{
		output = ShaderSourceMap.at(name);
	}
	return output;
}

bool AssetManager::GetShaderAsset(std::string path, ShaderAsset & asset)
{
	return false;
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

//todo: Check time stamps!
BaseTexture * AssetManager::DirectLoadTextureAsset(std::string name, bool DirectLoad, DeviceContext* Device)
{
	AssetPathRef Fileref = AssetPathRef(name);

	//todo: Deal with TGA to DDS 
	if (Fileref.GetFileType() == AssetFileType::DDS || name.find(".tga") != -1 || DirectLoad)
	{
		return RHI::CreateTexture(Fileref, Device);
	}

	
	//check the DDC for A Generated one
	std::string DDCRelFilepath = "\\" + DDCName + "\\" + Fileref.BaseName + ".DDS";
	Fileref.DDCPath = DDCRelFilepath;
	if (FileUtils::File_ExistsTest(GetRootDir() + DDCRelFilepath) && !PlatformApplication::CheckFileSrcNewer(GetRootDir() + DDCRelFilepath, Fileref.GetFullPathToAsset()))
	{
		Fileref.IsDDC = true;
		return RHI::CreateTexture(Fileref, Device);
	}
	else
	{
		//File is not a DDS
#if BUILD_PACKAGE
		Log::OutS << "File '" << Fileref.Name << "' Is missing from the cooked data" << Log::OutS;
#else
		Log::OutS << "File '" << Fileref.Name << "' Does not exist in the DDC Generating Now" << Log::OutS;
		//generate one! BC1_UNORM  
		std::string Args = " " + GetScriptPath();
		Args.append(" BC1_UNORM ");
		Args.append('"' + Fileref.GetFullPathToAsset() + '"' + " ");
		Args.append(GetDDCPath());
		PlatformApplication::ExecuteHostScript(GetTextureGenScript(), Args);
		if (FileUtils::File_ExistsTest(GetRootDir() + DDCRelFilepath))
		{
			Fileref.IsDDC = true;
			return RHI::CreateTexture(Fileref, Device);
		}
		else
		{
			Log::OutS << "File '" << Fileref.Name << "' Failed To Generate!" << Log::OutS;
		}
#endif
	}
	return ImageIO::GetDefaultTexture();
}

std::string AssetManager::LoadShaderIncludeFile(std::string name, int limit, std::string Relative)
{
	std::string file;
	limit++;
	if (limit > MaxIncludeTreeLength)
	{
		return file;
	}
	std::string pathname = name;
	if (limit > 0)
	{
		pathname = GetShaderPath();
		pathname.append(name);
	}
	if (!FileUtils::File_ExistsTest(pathname))
	{
		pathname = GetShaderPath();
		pathname.append(Relative);
		pathname.append(name);
	}
	std::string RelativeStartPath = "";
	if (name.find('\\') != -1)
	{
		RelativeStartPath = name;
		std::vector<std::string> data = StringUtils::Split(name, '\\');
		StringUtils::RemoveChar(RelativeStartPath, data[data.size() - 1]);
	}

	std::ifstream myfile(pathname);
	if (myfile.is_open())
	{
		std::string line;
		while (std::getline(myfile, line))
		{
			if (line.find("#") != -1)
			{
				size_t targetnum = line.find(includeText);
				if (targetnum != -1)
				{
					if (line.find("//") != -1)//Contains a commented line 
					{
						file.append(" \n");
						continue;
					}
					line.erase(targetnum, includeLength);
					StringUtils::RemoveChar(line, "\"");
					StringUtils::RemoveChar(line, "\"");

					file.append(LoadShaderIncludeFile(line.c_str(), limit, RelativeStartPath));
				}
				else
				{
					file.append(line);
				}
			}
			else
			{
				file.append(line);
			}
			file.append(" \n");
		}
		myfile.close();
	}
	else
	{
		Log::OutS << "failed to load " << pathname << Log::OutS;
	}
	return file;
}
