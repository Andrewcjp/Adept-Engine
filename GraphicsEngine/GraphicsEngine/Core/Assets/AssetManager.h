#pragma once
#include <map>
#include <string>
#include "../Core/Utils/StringUtil.h"
#include "Core/Engine.h"
class BaseTexture;
class ShaderProgramBase;


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
#include <d3d12.h>
struct ShaderAsset
{
	std::string Name;
	//rendersystem?
	union
	{
		int id;
		ID3DBlob* blob;
	};
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
	void LoadFromShaderDir();
	void LoadTexturesFromDir();
	bool FileExists(std::string filename);
	bool LoadTextureAsset();
	void LoadCookedShaders();
	void CookTextureAsset();
	void ExportCookedShaders();
	static void StartAssetManager();
	static AssetManager* instance;
	AssetManager();
	~AssetManager();

	bool GetTextureAsset(std::string path, TextureAsset & asset, bool ABSPath = false);
	static size_t GetShaderAsset(std::string name, char ** buffer);
	std::string LoadFileWithInclude(std::string name);
	std::string TextureAssetPath = "../asset/shader/glsl/";
	bool GetShaderAsset(std::string path, ShaderAsset &asset);
	std::map<std::string, TextureAsset>* GetTextureMap() { return &TextureAssetsMap; }
	std::map<std::string, std::string>* GetMeshMap() { return &MeshFileMap; }
	const std::string ShaderCookedFile = "\\asset\\Cooked\\Shader\\glsl\\CookedShaders.txt";
	static void RegisterMeshAssetLoad(std::string name);
	static std::string GetShaderDirPath();
	static BaseTexture * DirectLoadTextureAsset(std::string name, bool DirectLoad = false, class DeviceContext * Device = nullptr);
	enum AssetFileType
	{
		DDS,
		Image,//any format jpg, png etc.
		Mesh,
		Unknown,
		Count
	};
	struct AssetPathRef
	{
		AssetPathRef(std::string Filename)
		{
			std::vector<std::string> split = StringUtils::Split(Filename, '.');
			if (split.size() == 2)
			{
				Name = split[0];
				Extention = split[1];
				FileType = ParseAssetFileType(Extention);
			}
			split = StringUtils::Split(Name, '\\');
			if (split.size() > 0)
			{
				BaseName = split[split.size() - 1];
			}
		}
		const std::string GetFullPathToAsset()const
		{
			return Engine::GetRootDir() + Name + "."+Extention;
		}
		const std::string GetRelativePathToAsset()const
		{
			return Name + "." + Extention;
		}
		const std::string GetExtention()const
		{
			return Extention;
		}
		std::string Name;
		std::string BaseName;
		const AssetFileType GetFileType() const 
		{
			return FileType;
		}
	private:
		std::string Extention;
		AssetFileType FileType = AssetFileType::Unknown;
	};
	static AssetFileType ParseAssetFileType(std::string Data)
	{
		if (Data.find("dds") != -1|| Data.find("DDS") != -1)
		{
			return AssetFileType::DDS;
		}
		if (Data.find("jpg") != -1 || Data.find("png") != -1)
		{
			return AssetFileType::Image;
		}
		if (Data.find("obj") != -1 || Data.find("fbx") != -1)
		{
			return AssetFileType::Mesh;
		}
		return AssetFileType::Unknown;
	}
private:
	const std::string AssetRootPath = "../asset/";
	std::string ShaderAssetPath = "../asset/shader/glsl/";

	static const std::string GetDDCPath();
	static const std::string GetTextureGenScript();
	bool HasCookedData = false;
	size_t ReadShader(std::string path, char ** buffer);


	std::string LoadShaderIncludeFile(std::string name, int limit);
	size_t TextFileBufferedRead(std::string name, char ** buffer);
	bool UseIncluderToLoadOGLShaders = true;
	const char * includeText = "#include";
	int	 includeLength = 9;
	std::map<std::string, std::string> ShaderSourceMap;
	std::map<std::string, TextureAsset> TextureAssetsMap;
	std::map<std::string, ShaderAsset> ShaderMap;
	std::map<std::string, std::string> MeshFileMap;
	bool PreLoadTextShaders = true;
	bool UseCookedShaders = false;
	bool CookShaders = true;
	bool UseCookedtextures = true;

	std::string TextureCooked = "../asset/TextureCooked.bin";
	float StartTime = 0;
	size_t LoadedAssetSize = 0;
	std::string FileSplit = "|FileStart|";
};

