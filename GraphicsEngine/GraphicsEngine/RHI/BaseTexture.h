#pragma once
#include "../EngineGlobals.h"
class BaseTexture
{
public:
	virtual			~BaseTexture() { ; }
	virtual void Bind(int unit) = 0;
	virtual void Bind(CommandListDef* list)
	{
		UNUSED_PARAM(list);
	}
	virtual void FreeTexture() = 0;
	virtual void SetTextureID(int id) = 0;
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) = 0;
	virtual void					CreateTextureFromData(void* data, int type, int width, int height, int bits) = 0;
	const char * AssetName = "";
};