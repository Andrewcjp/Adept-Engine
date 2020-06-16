#pragma once
#include "Core/IRefCount.h"
#include "Core/EngineInc_fwd.h"
#include "Core/ObjectBase/SharedPtr.h"
#include "RHITemplates.h"

struct ETextureType
{
	enum Type
	{
		Type_2D,
		Type_CubeMap
	};
};

struct TextureDescription
{
	static TextureDescription DefaultTexture(int width, int height);
	static TextureDescription DefaultTextTexture(int width, int height);
	int Width = 0;
	int Height = 0;
	int BitDepth = 0;
	int MipLevels = 1;
	int Faces = 1;
	ETextureFormat Format = ETextureFormat::FORMAT_B8G8R8A8_UNORM;
	void* PtrToData = nullptr;
	ETextureType::Type TextureType = ETextureType::Type_2D;
	int ImageByteSize = 0;
	std::vector<glm::ivec2> MipLevelExtents;
	RHI_API uint64_t Size(int mip);
	RHI_API glm::ivec2 MipExtents(int mip);
	std::string Name = "";
};

class BaseTexture :  public IRHIResourse, public IRHISharedDeviceObject<BaseTexture>
{
public:
	CORE_API virtual ~BaseTexture();
	bool CreateFromFile(AssetPathRef FileName);
	static RHITexture * CreateFromFile2(AssetPathRef FileName);
	CORE_API void CreateTextureFromData(void* data, int type, int width, int height, int bits);
	virtual void CreateTextureFromDesc(const TextureDescription& desc) = 0;
	virtual void CreateAsNull() = 0;
	std::string TextureName;
	std::string TexturePath;
	CORE_API ETextureType::Type GetType();
	int MaxMip = -1;
	RHI_API DeviceContext* GetContext() const;
protected:
	ETextureType::Type CurrentTextureType = ETextureType::Type_2D;
	DeviceContext* Context = nullptr;
	TextureDescription Description;
};
typedef  SharedPtr<BaseTexture> BaseTextureRef;