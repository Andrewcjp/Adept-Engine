#pragma once
#include "../EngineGlobals.h"
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
	D3D12CBV();
	~D3D12CBV();
	void SetDescriptorHeaps(CommandListDef * list);
	void SetGpuView(CommandListDef* list, int offset, ShaderRegisterSlot slot = MainCBV);
	template<typename  T>
	void UpdateCBV(T& buffer, int offset)
	{
		memcpy(m_pCbvDataBegin + (offset * CB_Size), &buffer, sizeof(buffer));
	}
	void InitCBV(int StructSize,int Elementcount = 1);
private:
	ID3D12Resource* m_constantBuffer;
	UINT8* m_pCbvDataBegin;
	ID3D12DescriptorHeap* m_cbvHeap;
	int InitalBufferCount = 10;
	int CB_Size = 0;
};

