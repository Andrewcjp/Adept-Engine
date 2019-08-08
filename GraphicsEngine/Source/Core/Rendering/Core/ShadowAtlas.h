#pragma once
//contains Render targets for shadow maps to use.
class ShadowAtlas;
struct ShadowAtlasHandle
{
	FrameBuffer* DynamicMapPtr = nullptr;
	ShadowAtlas* AtlasNode = nullptr;
	Light* lightPtr = nullptr;
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
	void Init();
	std::map<Light*, ShadowAtlasHandle*> AllocatedHandles;
	std::vector<ShadowAtlasHandle*> DeallocatedHandles;
	RHITextureArray* ShadowCubeArray = nullptr;
	DeviceContext* Context = nullptr;
	int MaxPointLight = 4;
};

