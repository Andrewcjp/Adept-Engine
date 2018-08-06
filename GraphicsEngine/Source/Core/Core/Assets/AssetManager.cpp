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
void AssetManager::LoadTexturesFromDir()
{
	std::string path = TextureAssetPath;
	for (auto & p : std::experimental::filesystem::directory_iterator(path))
	{
		if ((p.path().filename().string().find(".") == -1))
		{
			continue;
		}
		TextureAsset t;
		GetTextureAsset(p.path().string(), t, true);
	}
}

void AssetManager::ExportCookedShaders()
{
	std::string dirtarget = ShaderCookedFile;
	StringUtils::RemoveChar(dirtarget, "CookedShaders.txt");
	FileUtils::CreateDirectoryFromFullPath(Engine::GetExecutionDir(), dirtarget, true);
	std::ofstream myfile(Engine::GetExecutionDir() + ShaderCookedFile);
	if (myfile.is_open())
	{
		for (std::map<std::string, std::string>::iterator it = ShaderSourceMap.begin(); it != ShaderSourceMap.end(); ++it)
		{
			//Log::OutS  << it->first << " => " << it->second << '\n';
			std::string data = "";
			data.append(it->first);
			data.append(FileSplit + "\n");
			data.append(it->second);
			data.append("¬\n");
			myfile.write(data.c_str(), data.length());
		}
	}
	myfile.close();
}
void AssetManager::LoadCookedShaders()
{
	std::ifstream myfile(Engine::GetExecutionDir() + ShaderCookedFile);
	std::string file;
	if (myfile.is_open())
	{
		std::string line;
		std::string shaderSource;
		std::string Key;
		while (std::getline(myfile, line))
		{
			if (line.find("¬") != -1)
			{
				std::string Shader = shaderSource;
				ShaderSourceMap.emplace(Key, Shader);
				shaderSource = "";
				Key = "";
				continue;
			}
			size_t target = line.find(FileSplit);
			if (target == -1)
			{
				shaderSource.append(line);
				shaderSource.append("\n");
				continue;
			}
			else
			{
				Key = line;
				StringUtils::RemoveChar(Key, FileSplit);
				StringUtils::RemoveChar(Key, "¬");
			}
		}
		myfile.close();
	}
}
//todo: Unload Shaders To save memory
//and wrap the code to recreate when reqeusted
//todo: image/mesh writing data
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

void AssetManager::SetupPaths()
{
	RootDir = Engine::GetExecutionDir();
	StringUtils::RemoveChar(RootDir, "\\Binaries");

	ContentDirPath = RootDir + "\\Content\\";
	if (!FileUtils::File_ExistsTest(ContentDirPath))
	{
		PlatformApplication::DisplayMessageBox("Error", "No Content Dir");
		Engine::Exit(-1);
	}
	ShaderDirPath = RootDir + "\\Shaders\\";
	if (!FileUtils::File_ExistsTest(ShaderDirPath))
	{
		PlatformApplication::DisplayMessageBox("Error", "No Content Dir");
		Engine::Exit(-1);
	}
	DDCDirPath = RootDir + "\\" + DDCName + "\\";
	PlatformApplication::TryCreateDirectory(DDCDirPath);

	TextureGenScriptPath = RootDir + "\\Scripts\\ConvertToDDS.bat";
	ScriptDirPath = RootDir + "\\Scripts\\";
	if (!FileUtils::File_ExistsTest(TextureGenScriptPath))
	{
		PlatformApplication::DisplayMessageBox("Error", "Texture Get Script Missing");
	}

}

AssetManager::AssetManager()
{
	SetupPaths();
	LoadFromShaderDir();
	PlatformApplication::TryCreateDirectory(GetDDCPath());
#if BUILD_PACKAGE
	if (PreLoadTextShaders)
	{
		if (FileExists(Engine::GetExecutionDir() + ShaderCookedFile))
		{
			LoadCookedShaders();
		}
		else
		{
			ExportCookedShaders();
		}
	}
	LoadTexturesFromDir();
#else
	//LoadCookedShaders();
	//ExportCookedShaders();
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

	//File is not a DDS
	//check the DDC for A Generated one
	std::string DDCRelFilepath = "\\" + DDCName + "\\" + Fileref.BaseName + ".DDS";
	Fileref.DDCPath = DDCRelFilepath;
	if (FileUtils::File_ExistsTest(GetRootDir() + DDCRelFilepath))
	{
		Fileref.IsDDC = true;
		return RHI::CreateTexture(Fileref, Device);
	}
	else
	{
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
	}
	return ImageIO::GetDefaultTexture();
}

std::string AssetManager::LoadShaderIncludeFile(std::string name, int limit)
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
					file.append(LoadShaderIncludeFile(line.c_str(), limit));
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


//Remove if unused 
#if 0
size_t AssetManager::GetShaderAsset(std::string name, char ** buffer)
{
	if (instance != nullptr)
	{
		return instance->ReadShader(name, buffer);
	}
	return 0;
}

size_t AssetManager::ReadShader(std::string name, char ** buffer)
{
	std::string NamePath(ShaderAssetPath);
	NamePath.append(name);

	return TextFileBufferedRead(NamePath, buffer);
}
size_t AssetManager::TextFileBufferedRead(std::string name, char** buffer)
{
	std::wstring newfile((int)name.size(), 0);
	MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), &newfile[0], (int)name.size());
	LPCWSTR filename = newfile.c_str();
	FILE *pfile = NULL;
	*buffer = NULL;
	size_t count = 0;

	if (filename == NULL) return 0;

	_wfopen_s(&pfile, filename, L"rt");

	if (!pfile) return 0;

	fseek(pfile, 0, SEEK_END);
	count = ftell(pfile);
	rewind(pfile);

	if (count > 0)
	{
		*buffer = new char[count + 1];

		count = (fread(*buffer, sizeof(char), count, pfile));
		(*buffer)[count] = '\0';
	}

	fclose(pfile);

	return count;
}
#endif