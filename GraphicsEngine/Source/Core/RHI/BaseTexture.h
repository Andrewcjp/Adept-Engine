#pragma once

#include "Core/IRefCount.h"
#include "RHI/RHITypes.h"
#include "Core/EngineInc_fwd.h"
class BaseTexture : public IRefCount, public IRHIResourse
{
public:
	enum ETextureType
	{
		Type_2D,
		Type_CubeMap
	};
	CORE_API virtual ~BaseTexture();
	virtual bool CreateFromFile(AssetPathRef FileName) = 0;
	virtual void CreateTextureFromData(void* data, int type, int width, int height, int bits) = 0;
	virtual void CreateAsNull() = 0;
	std::string  TextureName;
	std::string  TexturePath;
	ETextureType GetType()
	{
		return CurrentTextureType;
	}
	int MaxMip = -1;
	virtual void UpdateSRV() = 0;
protected:
	ETextureType CurrentTextureType = ETextureType::Type_2D;
};