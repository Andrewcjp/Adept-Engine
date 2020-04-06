#pragma  once

class TextureAsset;
class BaseTexture;
class RHITexture;
class MeshRendererComponent;
namespace EStreamPriority
{
	enum Type
	{
		Critial,
		High,
		Medium,
		Low
	};
}
static const int StreamAllValue = -1;
struct TextureStreamRequest
{
	BaseTexture* TargetTexture = nullptr;
	int TargetMip = StreamAllValue;
	int TargetFace = StreamAllValue;
	TextureAsset* Asset = nullptr;
	EStreamPriority::Type Priority = EStreamPriority::Medium;
	void Validate();
};

//represents a virtual texture
//It might be backed by data
class TextureHandle 
{
public:
	TextureHandle();
	~TextureHandle();
	void InitFromFile(std::string file);
	void LoadToCPUMemory();
	void LinkToMesh(MeshRendererComponent* Mesh);
	int TopMipState = 0;
	int TargetMip = 0;
	bool IsResident()const
	{
		return Resident;
	}
	void Bind(RHICommandList* List, std::string SlotName);
	void UnLoadFromCPUMemory();
	const TextureDescription& GetDesc()const { return Description; }
	std::string GetFilePath()const { return FilePath; }
private:
	bool IsCPULoaded = false;
	//todo: group!
	RHITexture* Backing = nullptr;
	MeshRendererComponent* LinkedMesh = nullptr;
	bool Resident = false;
	int MipCount = 9;
	std::string FilePath = "";
	TextureDescription Description;
	friend class DXGPUTextureStreamer;
	friend class GPUTextureStreamer;
};