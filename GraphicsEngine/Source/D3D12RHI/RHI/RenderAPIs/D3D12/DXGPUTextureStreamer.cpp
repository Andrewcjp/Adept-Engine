#include "DXGPUTextureStreamer.h"
#include "D3D12RHI.h"


DXGPUTextureStreamer::DXGPUTextureStreamer()
{}


DXGPUTextureStreamer::~DXGPUTextureStreamer()
{}

void DXGPUTextureStreamer::Tick()
{

}

void DXGPUTextureStreamer::OnInit(DeviceContext* con)
{
	Device = D3D12RHI::DXConv(con);
}
