#include "WindowsDx12DeviceInterface.h"
#include "../../D3D12DeviceContext.h"
#include "../../D3D12RHI.h"
#if SUPPORT_DXGI
WindowsDx12DeviceInterface* WindowsDx12DeviceInterface::Instance = nullptr;
static ConsoleVariable ForceGPUIndex("ForceDeviceIndex", -1, ECVarType::LaunchOnly, true);
static ConsoleVariable ForceSingleGPU("ForceSingleGPU", 0, ECVarType::LaunchOnly, false);
static ConsoleVariable AllowWarp("AllowWarp", 0, ECVarType::LaunchOnly, false);
WindowsDx12DeviceInterface::WindowsDx12DeviceInterface()
{

}


WindowsDx12DeviceInterface::~WindowsDx12DeviceInterface()
{}

void WindowsDx12DeviceInterface::CreateDevices(D3D12RHI * pRHI,bool debug) 
{
	Instance = new WindowsDx12DeviceInterface();
	Instance->RHIptr = pRHI;
	UINT dxgiFactoryFlags = 0;
	if (debug)
	{
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, ID_PASS(&Instance->factory)));
	if (!Instance->FindAdaptors(Instance->factory, false))//Search adapters in a picky fashion 
	{
		Log::LogMessage("Failed to find select device index, Defaulting", Log::Severity::Warning);
		Instance->FindAdaptors(Instance->factory, true);//force find an adapter
	}
}
#if 0
IDXGIAdapter* warpAdapter;
ThrowIfFailed(factory->EnumWarpAdapter(ID_PASS(&warpAdapter)));
DeviceContexts[1] = new D3D12DeviceContext();
DeviceContexts[1]->CreateDeviceFromAdaptor((IDXGIAdapter1*)warpAdapter, 1);
Log::LogMessage("Found D3D12 GPU debugger, Warp adapter is now used instead of second physical GPU");
#endif
bool WindowsDx12DeviceInterface::FindAdaptors(IDXGIFactory2 * pFactory, bool ForceFind)
{
	int TargetIndex = ForceGPUIndex.GetIntValue();
	bool ForcingIndex = (TargetIndex != -1);
	if (ForceFind)
	{
		ForcingIndex = false;
	}
	IDXGIAdapter1* adapter;
	int CurrentDeviceIndex = 0;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

#if BUILD_SHIPPING
		bool AllowSoft = false;
#else
		bool AllowSoft = AllowWarp.GetBoolValue();
#endif
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE && !AllowSoft)
		{
			if (TargetIndex == adapterIndex)
			{
				ForcingIndex = false;
			}
			adapter->Release();
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}
		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			if (ForcingIndex && adapterIndex != TargetIndex)
			{
				adapter->Release();
				continue;
			}
			D3D12DeviceContext** Device = nullptr;
			if (CurrentDeviceIndex >= MAX_GPU_DEVICE_COUNT)
			{
				return true;
			}
			Device = &RHIptr->DeviceContexts[CurrentDeviceIndex];
			if (*Device == nullptr)
			{
				*Device = new D3D12DeviceContext();
				//#SLI This needs to create more Devices 
				(*Device)->CreateDeviceFromAdaptor(adapter, CurrentDeviceIndex);
				CurrentDeviceIndex++;
				if (ForceSingleGPU.GetBoolValue())
				{
					Log::LogMessage("Forced Single Gpu Mode");
					return true;
				}
				if ((*Device)->IsPartOfNodeGroup())
				{
					//we have a linked group!
					//
					int ExtraNodes = glm::min(CurrentDeviceIndex + (*Device)->GetNodeCount() - 1, MAX_GPU_DEVICE_COUNT);
					for (int i = 0; i < ExtraNodes; i++)
					{
						RHIptr->DeviceContexts[CurrentDeviceIndex] = new D3D12DeviceContext();
						RHIptr->DeviceContexts[CurrentDeviceIndex]->CreateNodeDevice((*Device)->GetDevice(), i + 1, CurrentDeviceIndex);
						CurrentDeviceIndex++;
					}
				}
			}
		}
	}
	return (CurrentDeviceIndex != 0);
}

#endif