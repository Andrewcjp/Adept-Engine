#pragma once
#include "BaseAsset.h"
UCLASS()
class MaterialAsset : public BaseAsset
{
public:
	CLASS_BODY_Reflect();
	MaterialAsset();

	PROPERTY();
	std::string ShaderAssetPath = "";
};

