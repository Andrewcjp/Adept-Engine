#include "stdafx.h"
#include "D3D12TimeManager.h"


D3D12TimeManager::D3D12TimeManager()
{}


D3D12TimeManager::~D3D12TimeManager()
{}
void D3D12TimeManager::Initialize(uint32_t MaxNumTimers)
{
	uint64_t GpuFrequency;
	D3D12RHI::Instance->GetCommandQueue()->GetTimestampFrequency(&GpuFrequency);

	sm_GpuTickDelta = 1.0 / static_cast<double>(GpuFrequency);

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_READBACK;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC BufferDesc;
	BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	BufferDesc.Alignment = 0;
	BufferDesc.Width = sizeof(uint64_t) * MaxNumTimers * 2;
	BufferDesc.Height = 1;
	BufferDesc.DepthOrArraySize = 1;
	BufferDesc.MipLevels = 1;
	BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	BufferDesc.SampleDesc.Count = 1;
	BufferDesc.SampleDesc.Quality = 0;
	BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &BufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&sm_ReadBackBuffer)));
	sm_ReadBackBuffer->SetName(L"GpuTimeStamp Buffer");

	D3D12_QUERY_HEAP_DESC QueryHeapDesc;
	QueryHeapDesc.Count = MaxNumTimers * 2;
	QueryHeapDesc.NodeMask = 1;
	QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&sm_QueryHeap)));
	sm_QueryHeap->SetName(L"GpuTimeStamp QueryHeap");

	sm_MaxNumTimers = (uint32_t)MaxNumTimers;
}
//void D3D12TimeManager::BeginReadBack(void)
//{
//	Graphics::g_CommandManager.WaitForFence(sm_Fence);
//
//	D3D12_RANGE Range;
//	Range.Begin = 0;
//	Range.End = (sm_NumTimers * 2) * sizeof(uint64_t);
//	ASSERT_SUCCEEDED(sm_ReadBackBuffer->Map(0, &Range, reinterpret_cast<void**>(&sm_TimeStampBuffer)));
//
//	sm_ValidTimeStart = sm_TimeStampBuffer[0];
//	sm_ValidTimeEnd = sm_TimeStampBuffer[1];
//
//	// On the first frame, with random values in the timestamp query heap, we can avoid a misstart.
//	if (sm_ValidTimeEnd < sm_ValidTimeStart)
//	{
//		sm_ValidTimeStart = 0ull;
//		sm_ValidTimeEnd = 0ull;
//	}
//}
//
//void D3D12TimeManager::EndReadBack(void)
//{
//	// Unmap with an empty range to indicate nothing was written by the CPU
//	D3D12_RANGE EmptyRange = {};
//	sm_ReadBackBuffer->Unmap(0, &EmptyRange);
//	sm_TimeStampBuffer = nullptr;
//}
//
//float D3D12TimeManager::GetTime(uint32_t TimerIdx)
//{
//	/*ASSERT(sm_TimeStampBuffer != nullptr, "Time stamp readback buffer is not mapped");
//	ASSERT(TimerIdx < sm_NumTimers, "Invalid GPU timer index");*/
//
//	uint64_t TimeStamp1 = sm_TimeStampBuffer[TimerIdx * 2];
//	uint64_t TimeStamp2 = sm_TimeStampBuffer[TimerIdx * 2 + 1];
//
//	if (TimeStamp1 < sm_ValidTimeStart || TimeStamp2 > sm_ValidTimeEnd || TimeStamp2 <= TimeStamp1)
//		return 0.0f;
//
//	return static_cast<float>(sm_GpuTickDelta * (TimeStamp2 - TimeStamp1));
//}
