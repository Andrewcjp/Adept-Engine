#pragma once
#include <d3d12.h>
class D3D12CBV
{
public:
	enum ShaderRegisterSlot
	{
		MainCBV = 1,
		LightCBV = 2,
		MPCBV = 3
	};
	D3D12CBV(class DeviceContext* Device);
	~D3D12CBV();
	void SetDescriptorHeaps(ID3D12GraphicsCommandList * list);
	void SetGpuView(ID3D12GraphicsCommandList * list, int offset, int slot = MainCBV, bool IsCompute = false);
	template<typename  T>
	void UpdateCBV(T& buffer, int offset)
	{
		memcpy(m_pCbvDataBegin + (offset * CB_Size), &buffer, sizeof(buffer));
	}
	void UpdateCBV(void* buffer, int offset, int size)
	{
		memcpy(m_pCbvDataBegin + (offset * CB_Size), buffer, size);
	}
	void InitCBV(int StructSize, int Elementcount = 1);
private:
	ID3D12Resource * m_constantBuffer;
	UINT8* m_pCbvDataBegin;
	ID3D12DescriptorHeap* m_cbvHeap;
	int InitalBufferCount = 10;
	int CB_Size = 0;
	class D3D12DeviceContext* Device = nullptr;
};

