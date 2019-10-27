#pragma once
#include "Core/IRefCount.h"
#include "Core/EngineInc_fwd.h"
#include "Core/ObjectBase/SharedPtr.h"
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
	eTEXTURE_FORMAT Format = eTEXTURE_FORMAT::FORMAT_B8G8R8A8_UNORM;
	void* PtrToData = nullptr;
	ETextureType::Type TextureType = ETextureType::Type_2D;
	int ImageByteSize = 0;
};

class BaseTexture :  public IRHIResourse, public IRHISharedDeviceObject<BaseTexture>
{
public:
	CORE_API virtual ~BaseTexture();
	virtual bool CreateFromFile(AssetPathRef FileName) = 0;
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