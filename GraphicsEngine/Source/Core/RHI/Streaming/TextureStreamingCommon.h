#pragma  once
#include "../BaseTexture.h"

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
namespace EGPUSteamMode
{
	enum Type
	{
		None,				//load texture all to memory no tiled resource support
		TiledTexture,		//load texture using tile streaming to full mips/CPU relevance
		TopMipUsedBased,	//Stream tiles based on Top mip GPU used
		SamplerFeedBack,	//Stream tiles based on Sampler feedback of tiles used.
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
	struct PerGPUData
	{
		RHITexture* Backing = nullptr;
		int TopMipState = 0;
		int TargetMip = 0;
	};
	TextureHandle();
	~TextureHandle();
	void InitFromFile(std::string file);
	void LoadToCPUMemory();
	void LinkToMesh(MeshRendererComponent* Mesh);
	RHIViewDesc GetCurrentView(int List);
	bool IsResident()const
	{
		return Resident;
	}
	void Bind(RHICommandList* List, std::string SlotName);
	void UnLoadFromCPUMemory();
	const TextureDescription& GetDesc()const { return Description; }
	std::string GetFilePath()const { return FilePath; }
	RHI_API PerGPUData* GetData(RHICommandList* list);
	RHI_API PerGPUData* GetData(DeviceContext* device);
	void SetAllTargetMip(uint64 mip)
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			GpuData[i].TargetMip = mip;
		}
	}
	bool IsValid()const { return ValidHandle; }
private:
	bool ValidHandle = false;
	PerGPUData GpuData[MAX_GPU_DEVICE_COUNT];
	bool IsCPULoaded = false;
	
	MeshRendererComponent* LinkedMesh = nullptr;
	bool Resident = false;
	int MipCount = 9;
	std::string FilePath = "";
	TextureDescription Description;
	friend class DXGPUTextureStreamer;
	friend class GPUTextureStreamer;
};