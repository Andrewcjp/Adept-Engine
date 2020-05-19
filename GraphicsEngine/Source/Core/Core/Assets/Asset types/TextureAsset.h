#pragma once
#include "BaseAsset.h"
#include "Core/Reflection/IReflect.h"
UCLASS();
class TextureAsset : public BaseAsset
{
public:
	CLASS_BODY_Reflect();
	TextureAsset();
	~TextureAsset();

};

