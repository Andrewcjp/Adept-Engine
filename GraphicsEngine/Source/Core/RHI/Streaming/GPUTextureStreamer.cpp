#include "GPUTextureStreamer.h"
#include "TextureStreamingEngine.h"


GPUTextureStreamer::GPUTextureStreamer()
{}


GPUTextureStreamer::~GPUTextureStreamer()
{}

void GPUTextureStreamer::Init(DeviceContext * Con)
{
	list = RHI::CreateCommandList(ECommandListType::Graphics, Con);
	TextureStreamingEngine::Get()->RegisterGPUStreamer(this);
	OnInit(Con);
}

void GPUTextureStreamer::Update()
{
	Tick(list);
}

void GPUTextureStreamer::RealiseHandle(TextureHandle* handle)
{
	ensure(handle->IsCPULoaded)
	Handles.push_back(handle);
	OnRealiseTexture(handle);
}

void GPUTextureStreamer::SetTargetSize(uint64 size)
{
	TargetPoolSize = size;
}

void GPUTextureStreamer::Tick(RHICommandList* list)
{}

void GPUTextureStreamer::OnInit(DeviceContext* Con)
{

}
