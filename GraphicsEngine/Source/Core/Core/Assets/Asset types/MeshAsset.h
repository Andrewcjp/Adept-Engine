#pragma once
#include "BaseAsset.h"
UCLASS();
class MeshAsset : public BaseAsset
{
public:
	CLASS_BODY_Reflect();
	MeshAsset();
	~MeshAsset();
};

