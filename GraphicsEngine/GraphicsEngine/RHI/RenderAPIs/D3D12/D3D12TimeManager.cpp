#include "stdafx.h"
#include "D3D12TimeManager.h"
#include "../RHI/DeviceContext.h"
#include "D3D12CommandList.h"

D3D12TimeManager* D3D12TimeManager::Instance = nullptr;
D3D12TimeManager::D3D12TimeManager()
{
	
}


D3D12TimeManager::~D3D12TimeManager()
{}
void D3D12TimeManager::Initialize(DeviceContext* context)
{
	if (Instance == nullptr)
	{
		Instance = new D3D12TimeManager();
		Instance->Init(context);
	}
}

void D3D12TimeManager::Destory()
{
	if (Instance != nullptr)
	{
		delete Instance;
	}
}

void D3D12TimeManager::Init(DeviceContext* context)
{
	// Create query heaps and result buffers.
	{
		int GraphicsAdaptersCount = 1;

		// Two timestamps for each frame.
		const UINT resultCount = 2 * 3;//sync with count;
		const UINT resultBufferSize = resultCount * sizeof(UINT64);

		D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
		timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		timestampHeapDesc.Count = resultCount;

		for (UINT i = 0; i < GraphicsAdaptersCount; i++)
		{
			ThrowIfFailed(context->GetDevice()->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_timestampResultBuffers)));

			ThrowIfFailed(context->GetDevice()->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&m_timestampQueryHeaps)));
		}
	}
	ThrowIfFailed(context->GetCommandQueue()->GetTimestampFrequency(&m_directCommandQueueTimestampFrequencies));
}
void D3D12TimeManager::UpdateTimers()
{
	D3D12_RANGE readRange = {};
	const D3D12_RANGE emptyRange = {};
	void* pData = nullptr;
	ThrowIfFailed(m_timestampResultBuffers->Map(0, &readRange, &pData));

	const UINT64* pTimestamps = reinterpret_cast<UINT64*>(static_cast<UINT8*>(pData) + readRange.Begin);
	const UINT64 timeStampDelta = pTimestamps[1] - pTimestamps[0];

	// Unmap with an empty range (written range).
	m_timestampResultBuffers->Unmap(0, &emptyRange);

	gpuTimeUS = (timeStampDelta * 1000000) / m_directCommandQueueTimestampFrequencies;
	gpuTimeMS = (float)(timeStampDelta * 1000) / (float)m_directCommandQueueTimestampFrequencies;
	avg.Add(gpuTimeMS);
	AVGgpuTimeMS = avg.GetCurrentAverage();
}

void D3D12TimeManager::StartTimer(ID3D12GraphicsCommandList * ComandList)
{
	ComandList->EndQuery(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 0);
}

void D3D12TimeManager::StartTimer(RHICommandList* ComandList)
{
	D3D12CommandList* List = (D3D12CommandList*)ComandList;
	StartTimer(List->GetCommandList());
}

void D3D12TimeManager::EndTimer(ID3D12GraphicsCommandList* ComandList)
{
	ComandList->EndQuery(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 1);
	ComandList->ResolveQueryData(m_timestampQueryHeaps, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, m_timestampResultBuffers, 0);
}

void D3D12TimeManager::EndTimer(RHICommandList* ComandList)
{
	D3D12CommandList* List = (D3D12CommandList*)ComandList;
	EndTimer(List->GetCommandList());

}