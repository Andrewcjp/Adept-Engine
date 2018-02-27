#pragma once
#include <map>
#include <string>
class BaseTexture;
class ShaderProgramBase;
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
private:
	const std::string AssetRootPath = "../asset/";
	std::string ShaderAssetPath = "../asset/shader/glsl/";	
	
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

