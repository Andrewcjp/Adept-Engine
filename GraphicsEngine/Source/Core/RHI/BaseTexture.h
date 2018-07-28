#pragma once
#include "EngineGlobals.h"
#include "Core/IRefCount.h"
#include "Core/Assets/AssetManager.h"
class BaseTexture : public IRefCount
{
public:
	enum ETextureType
	{
		Type_2D,
		Type_CubeMap
	};
	virtual		 ~BaseTexture() { ; }
	virtual bool CreateFromFile(AssetPathRef FileName) = 0;	
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) = 0;
	virtual void CreateTextureFromData(void* data, int type, int width, int height, int bits) = 0;	
	virtual void CreateAsNull() =0;
	std::string  TextureName;
	std::string  TexturePath;
protected:
	ETextureType CurrentTextureType = ETextureType::Type_2D;
};