
class D3D12RHI;
#pragma once
#if SUPPORT_DXGI
class WindowsDx12DeviceInterface
{
public:
	WindowsDx12DeviceInterface();
	~WindowsDx12DeviceInterface();
	IDXGIFactory4 * factory = nullptr;
	static IDXGIFactory4 * GetFactory() { return Instance->factory; }
	static void CreateDevices(D3D12RHI* pRHI, bool debug);
	bool FindAdaptors(IDXGIFactory2 * pFactory, bool ForceFind);
private:

	static WindowsDx12DeviceInterface* Instance;
	D3D12RHI* RHIptr = nullptr;
};
#endif

