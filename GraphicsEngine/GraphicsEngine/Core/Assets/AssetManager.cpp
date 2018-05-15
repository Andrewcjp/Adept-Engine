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
#include "../Utils/StringUtil.h"
#include "../Core/Performance/PerfManager.h"
#include <d3d12.h>
#include "../Core/Utils/FileUtils.h"
void AssetManager::LoadFromShaderDir()
{
	std::string path = ShaderAssetPath;
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
		GetTextureAsset(p.path().string(), t,true);
	}
}
bool AssetManager::FileExists(std::string filename) 
{
	struct stat fileInfo;
	return stat(filename.c_str(), &fileInfo) == 0;
}
bool AssetManager::LoadTextureAsset()
{
	if (FileExists(TextureCooked) == false)
	{
		return false;
	}
	int datalength = 8;
	char * attribdata = new char[datalength];//two points of data
	std::ifstream ifp(TextureCooked, std::ios::in | std::ios::binary);
	ifp.read(reinterpret_cast<char*>(attribdata), datalength * sizeof(char));
	std::string data = attribdata;
	int count = std::stoi(data);
	for (int i = 0; i < count; i++)
	{
		TextureAsset Target;
		ifp.read(reinterpret_cast<char*>(&Target), sizeof(TextureAsset));
		ifp.read((char*)(Target.image), (Target.ByteSize));
		TextureAssetsMap.emplace(Target.name, Target);
	}
	ifp.close();
	return true;
}
void AssetManager::CookTextureAsset()
{
	std::ofstream ofp(TextureCooked, std::ios::out | std::ios::binary);
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(8) << (TextureAssetsMap.size());// << "|" << std::setfill('0') << std::setw(8) << (0);
	std::string data = stream.str();
	ofp.write((const char*)(data.c_str()), data.length());
	for (std::map<std::string, TextureAsset>::iterator it = TextureAssetsMap.begin(); it != TextureAssetsMap.end(); ++it)
	{
		/*	TextureAsset Target;
			GetTextureAsset("../asset/texture/grasshillalbedo.png", Target);*/
		ofp.write((const char*)(&it->second), sizeof(TextureAsset));
		//ofp.write((const char*)(it->second.name.data()), sizeof(it->second));
		ofp.write((const char*)(it->second.image), (it->second.ByteSize));
	}
	/*TextureAsset Target;
	GetTextureAsset("../asset/texture/grasshillalbedo.png", Target);
	ofp.write((const char*)(&Target), sizeof(TextureAsset));
	ofp.write((const char*)(Target.image), (Target.ByteSize));*/
	ofp.close();
	//ofp.write(reinterpret_cast<const char*>(&ShaderSourceMap), ShaderSourceMap.size() * sizeof(Vertex));
}

void AssetManager::ExportCookedShaders()
{
	std::string dirtarget = ShaderCookedFile;
	StringUtils::RemoveChar(dirtarget, "CookedShaders.txt");
	FileUtils::CreateDirectoryFromFullPath(Engine::GetRootDir(), dirtarget, true);
	std::ofstream myfile(Engine::GetRootDir()+ShaderCookedFile);
	if (myfile.is_open())
	{
		for (std::map<std::string, std::string>::iterator it = ShaderSourceMap.begin(); it != ShaderSourceMap.end(); ++it)
		{
			//std::cout << it->first << " => " << it->second << '\n';
			std::string data = "";
			data.append(it->first);
			data.append(FileSplit+"\n");
			data.append(it->second);
			data.append("¬\n");
			myfile.write(data.c_str(), data.length());
		}
	}
	myfile.close();
}
void AssetManager::LoadCookedShaders()
{
	std::ifstream myfile(Engine::GetRootDir()+ShaderCookedFile);
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
//todo: iamge/mesh writing data
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
AssetManager::AssetManager()
{
	StartTime = (float)PerfManager::get_nanos();
	std::string path = Engine::GetRootDir();	
	path.append("\\asset\\");
	/*StringUtils::RemoveChar(path, "|");*/
	ShaderAssetPath = path;
	ShaderAssetPath.append("shader\\hlsl\\");
	TextureAssetPath = path;
	TextureAssetPath.append("texture\\");
	LoadFromShaderDir();
#if BUILD_PACKAGE
	if (PreLoadTextShaders)
	{
		if (FileExists(Engine::GetRootDir() + ShaderCookedFile))
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
	ExportCookedShaders();
#endif
	std::cout << "Shaders Loaded in " << ((PerfManager::get_nanos() - StartTime) / 1e6f) << "ms " << std::endl;
	std::cout << "Texture Asset Memory " << (float)LoadedAssetSize / 1e6f << "mb " << std::endl;
}
AssetManager::~AssetManager()
{
}
#include "ImageIO.h"
bool AssetManager::GetTextureAsset(std::string path, TextureAsset &asset,bool ABSPath)
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
				fullpath = Engine::GetRootDir();
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
				std::cout << "Load texture Error " << fullpath << std::endl;
				return false;
			}
			newasset.image = image;
			newasset.ByteSize = (newasset.Width* newasset.Height) *(newasset.Nchannels * sizeof(unsigned char));
			LoadedAssetSize += newasset.ByteSize;
			StringUtils::RemoveChar(fullpath, Engine::GetRootDir());
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

std::string AssetManager::GetShaderDirPath()
{
	if (instance)
	{
		return instance->ShaderAssetPath;
	}
	return std::string();
}


std::string AssetManager::LoadShaderIncludeFile(std::string name, int limit)
{
	std::string file;
	limit++;
	if (limit > 2)
	{
		return file;
	}
	std::string pathname = name;
	if (limit > 0)
	{
		pathname = ShaderAssetPath;
		pathname.append(name);
	}
	std::ifstream myfile(pathname);
	if (myfile.is_open())
	{
		std::string line;
		while (std::getline(myfile, line))
		{
			///*	if (line.find("/*") != -1 || line.find("*/") != -1|| line.find("//") != -1)
			//	{
			//		continue;
			//	}*/
			if (line.find("#") != -1)
			{
				size_t targetnum = line.find(includeText);

				if (targetnum != -1)
				{
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
		std::cout << "failed to load " << pathname << std::endl;
	}
	return file;
}
size_t AssetManager::TextFileBufferedRead(std::string name, char** buffer)
{

	//std::string filename(file);
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