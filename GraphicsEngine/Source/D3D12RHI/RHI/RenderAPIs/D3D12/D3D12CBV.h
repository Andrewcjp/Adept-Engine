#pragma once

class DXDescriptor;
#define Enable_CBV_BUFFERING 1
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
	void SetGpuView(ID3D12GraphicsCommandList * list, int offset, int slot = MainCBV, bool IsCompute = false);
	template<typename  T>
	void UpdateCBV(T& buffer, int offset)
	{
		UpdateCBV(buffer, offset, sizeof(buffer));
	}
	void UpdateCBV(void* buffer, int offset, int size);
	void InitCBV(int StructSize, int Elementcount = 1);
	void SetName(LPCWSTR name);
private:
	GPUResource * m_constantBuffer;
	UINT8* m_pCbvDataBegin;
	int InitalBufferCount = 10;
	int CB_Size = 0;
	class D3D12DeviceContext* Device = nullptr;
	int SizeInBytes = 0;
	int RawStuctSize = 0;
};

