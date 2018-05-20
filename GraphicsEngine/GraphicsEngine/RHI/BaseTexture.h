#pragma once
#include "../EngineGlobals.h"
class BaseTexture
{
public:
	enum ETextureType
	{
		Type_2D,
		Type_CubeMap
	};
	virtual			~BaseTexture() { ; }
	virtual void Bind(int unit) = 0;
	virtual void Bind(CommandListDef* list)
	{
		UNUSED_PARAM(list);
	}
	virtual void FreeTexture() = 0;
	virtual void SetTextureID(int id) = 0;
	virtual bool CreateFromFile(std::string FileName) = 0;
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) = 0;
	virtual void CreateTextureFromData(void* data, int type, int width, int height, int bits) = 0;
	std::string TextureName;

	//Shared Refs
	void RemoveRef() { refcount--; };
	void AddRef() { refcount++; };
	int GetRefCount()
	{
		return refcount;
	};

protected:
	ETextureType CurrentTextureType = ETextureType::Type_2D;
private:
	int refcount = 0;
};