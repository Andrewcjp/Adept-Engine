#pragma once
class GPUTextureStreamer
{
public:
	RHI_API GPUTextureStreamer();
	RHI_API virtual ~GPUTextureStreamer();
	RHI_API void Init(DeviceContext* Con);
	void Update();

protected:
	RHI_API virtual void Tick();
	RHI_API virtual void OnInit(DeviceContext* con);
};

