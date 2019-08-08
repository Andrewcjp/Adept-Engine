#include "ShadowAtlas.h"
#include "Light.h"
#include "ShadowRenderer.h"

ShadowAtlas::ShadowAtlas(DeviceContext* device)
{
	Context = device;
	Init();
}

void ShadowAtlas::Init()
{
	ShadowCubeArray = RHI::CreateTextureArray(Context, MaxPointLight);
	ShadowCubeArray->SetFrameBufferFormat(ShadowRenderer::GetCubeMapFBDesc(1));
}

ShadowAtlas::~ShadowAtlas()
{}

ShadowAtlasHandle* ShadowAtlas::AllocateHandle(Light* l, DeviceContext* device)
{
	ShadowAtlasHandle* handle = new ShadowAtlasHandle();
	handle->lightPtr = l;
	AllocatedHandles.emplace(l, handle);
	//#Shadow: check space?
	//#Shadow: Reuse Old allocations
	//#Shadow: use single RT with multiple sections
	AllocateRenderTarget(handle, device);
	return handle;
}

bool ShadowAtlas::ReleaseHandle(ShadowAtlasHandle* handle)
{
	auto itor = AllocatedHandles.find(handle->lightPtr);
	if (itor != AllocatedHandles.end())
	{
		DeallocatedHandles.push_back(itor->second);
		AllocatedHandles.erase(itor);
		return true;
	}
	return false;
}

void ShadowAtlas::AllocateRenderTarget(ShadowAtlasHandle * handle, DeviceContext* dev)
{
	Light* lightptr = handle->lightPtr;
	RHIFrameBufferDesc desc;
	int size = glm::min(handle->lightPtr->Resolution, RHI::GetRenderSettings()->GetShadowSettings().MaxShadowMapSize);
	if (handle->lightPtr->GetLightMobility() == ELightMobility::Baked)
	{
		size = lightptr->BakedResolution;
	}
	if (handle->lightPtr->GetType() == ELightType::Point)
	{
		handle->DynamicMapPtr = RHI::CreateFrameBuffer(dev, ShadowRenderer::GetCubeMapFBDesc(size));
		ShadowCubeArray->AddFrameBufferBind(handle->DynamicMapPtr, lightptr->GetShadowId());
	}
	//if (lightPtr->GetLightMobility() == ELightMobility::Baked)
	//{
	//	StaticShadowMap[index] = ShadowMap[index];
	//}
}

void ShadowAtlas::BindPointmaps(RHICommandList * list, int slot)
{
	ShadowCubeArray->BindToShader(list, slot);
}

