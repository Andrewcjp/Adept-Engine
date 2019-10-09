#pragma once
//contains Render targets for shadow maps to use.
class ShadowAtlas;
class Light;
struct ShadowAtlasHandle
{
	FrameBuffer* DynamicMapPtr = nullptr;
	ShadowAtlas* AtlasNode = nullptr;
	Light* lightPtr = nullptr;
	int HandleId = 0;
};
class ShadowAtlas
{
public:
	ShadowAtlas(DeviceContext* device);
	~ShadowAtlas();
	ShadowAtlasHandle* AllocateHandle(Light* l, DeviceContext* device);
	bool ReleaseHandle(ShadowAtlasHandle* handle);
	void AllocateRenderTarget(ShadowAtlasHandle * handle, DeviceContext* dev);
	void BindPointmaps(RHICommandList * list, int slot);
private:
	void Destory();
	void Init();
	std::map<Light*, ShadowAtlasHandle*> AllocatedHandles;
	std::vector<ShadowAtlasHandle*> DeallocatedHandles;
	RHITextureArray* ShadowCubeArray = nullptr;
	DeviceContext* Context = nullptr;
	int MaxPointLight = 4;
};

