#include "D3D12RHIPCH.h"
#include "D3D12QueryHeap.h"
#include "D3D12CommandList.h"
#include "D3D12Query.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h"

D3D12QueryHeap::D3D12QueryHeap(D3D12DeviceContext * device, int MaxSize, D3D12_QUERY_HEAP_TYPE type)
{
	Device = device;
	HeapType = type;
	HeapSize = MaxSize;
	CreateHeap();
	CreateResultsBuffer();
}

D3D12QueryHeap::~D3D12QueryHeap()
{
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		SafeRelease(Buffers[i].QueryHeap);
		SafeRelease(Buffers[i].ResultBuffer);
	}
}

void D3D12QueryHeap::BeginQuery(D3D12CommandList * list, D3D12Query * Q)
{
	if (CurrentBatch.Open == false && HeapType == D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS)
	{
		BeginQuerryBatch();
	}
	ensureMsgf(D3D12Helpers::IsValidForQueryHeap(HeapType, Q->GetQueryType()), "Querry Type not valid for heap");
	//only for occlusion Query
	CurrentBatch.Alloc(Q);
	list->GetCommandList()->BeginQuery(GetHeap(), GetType(Q->GetQueryType()), Q->IndexInHeap);
}

void D3D12QueryHeap::EndQuerry(D3D12CommandList * list, D3D12Query * Q)
{
	ensureMsgf(D3D12Helpers::IsValidForQueryHeap(HeapType, Q->GetQueryType()), "Querry Type not valid for heap");
	CurrentBatch.Alloc(Q);
	list->GetCommandList()->EndQuery(GetHeap(), GetType(Q->GetQueryType()), Q->IndexInHeap);
}
void D3D12QueryHeap::BeginQuerryBatch()
{
	//ensure(!CurrentBatch.Open);
	CurrentBatch.Open = true;
	CurrentBatch.Reset();
}

void D3D12QueryHeap::ResolveAndEndQueryBatches(D3D12CommandList * list)
{
	if (HeapType == D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP)
	{
		ensure(list->IsCopyList());
	}
	ensure(CurrentBatch.Open);
	list->GetCommandList()->ResolveQueryData(GetHeap(), GetType(CurrentBatch.BatchType), 0, CurrentBatch.Count, GetBuffer(), 0);
	CurrentBatch.Open = false;
}

void D3D12QueryHeap::ReadData()
{
	D3D12_RANGE readRange = {};
	const D3D12_RANGE emptyRange = {};
	void* pData = nullptr;
	ThrowIfFailed(Buffers[Device->GetCpuFrameIndex()].ResultBuffer->Map(0, &readRange, &pData));
	UINT8* DataPtr = static_cast<UINT8*>(pData);
	const UINT64* pTimestamps = reinterpret_cast<UINT64*>(DataPtr + readRange.Begin);
	UINT8* CurrentDataPtr = DataPtr + readRange.Begin;
	for (int i = 0; i < CurrentBatch.Queries.size(); i++)
	{
		if (CurrentBatch.BatchType == EGPUQueryType::Timestamp)
		{
			CurrentBatch.Queries[i]->Result = pTimestamps[CurrentBatch.Queries[i]->IndexInHeap];
		}
		else if(CurrentBatch.BatchType == EGPUQueryType::Pipeline_Stats)
		{
			memcpy(&CurrentBatch.Queries[i]->PipeStatResult, CurrentDataPtr + i*GetHeapTypeSize(), GetHeapTypeSize());
		}
		CurrentBatch.Queries[i]->IsResolved = true;
	}
	Buffers[Device->GetCpuFrameIndex()].ResultBuffer->Unmap(0, &emptyRange);
}
UINT64 D3D12QueryHeap::GetHeapTypeSize()
{
	if (HeapType == D3D12_QUERY_HEAP_TYPE_TIMESTAMP || HeapType == D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP)
	{
		return sizeof(UINT64);
	}
	if (HeapType == D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS)
	{
		return sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);
	}
	return 1;
}
void D3D12QueryHeap::CreateHeap()
{
	const UINT resultCount = HeapSize;//sync with count;
	const UINT resultBufferSize = resultCount * GetHeapTypeSize();

	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
		timestampHeapDesc.Type = HeapType;
		timestampHeapDesc.Count = resultCount;
		ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&Buffers[i].ResultBuffer)));

		ThrowIfFailed(Device->GetDevice()->CreateQueryHeap(&timestampHeapDesc, IID_PPV_ARGS(&Buffers[i].QueryHeap)));
	}

}

void D3D12QueryHeap::CreateResultsBuffer()
{
	const UINT resultCount = HeapSize;//sync with count;
	const UINT resultBufferSize = resultCount * GetHeapTypeSize();
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		D3D12_QUERY_HEAP_DESC timestampHeapDesc = {};
		timestampHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		timestampHeapDesc.Count = resultCount;
		ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(resultBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&Buffers[i].ResultBuffer)));
	}
}

ID3D12QueryHeap * D3D12QueryHeap::GetHeap(int index/* = -1*/)
{
	if (index == -1)
	{
		index = Device->GetCpuFrameIndex();
	}

	return Buffers[index].QueryHeap;
}

ID3D12Resource * D3D12QueryHeap::GetBuffer(int i /*= -1*/)
{
	if (i == -1)
	{
		i = Device->GetCpuFrameIndex();
	}
	return Buffers[i].ResultBuffer;
}

D3D12_QUERY_TYPE D3D12QueryHeap::GetType(EGPUQueryType::Type qtype)
{
	if (HeapType == D3D12_QUERY_HEAP_TYPE_TIMESTAMP || HeapType == D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP)
	{
		return D3D12_QUERY_TYPE_TIMESTAMP;
	}
	else if (HeapType == D3D12_QUERY_HEAP_TYPE_OCCLUSION)
	{
		if (qtype == EGPUQueryType::BinaryOcclusion)
		{
			return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
		}
		return D3D12_QUERY_TYPE_OCCLUSION;
	}
	else if (HeapType == D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS)
	{
		return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
	}
	return D3D12_QUERY_TYPE();
}

void DX12QueryBatch::Reset()
{
	Startindex = 0;
	Count = 0;
	MemoryUtils::DeleteVector(Queries);
	Queries.clear();
}

void DX12QueryBatch::Alloc(D3D12Query* Q)
{

	if (BatchType == EGPUQueryType::Limit)
	{
		BatchType = Q->GetQueryType();
	}
	ensure(BatchType == Q->GetQueryType());
	if (Q->IndexInHeap > -1)
	{
		return;
	}
	Queries.push_back(Q);
	Q->IndexInHeap = Startindex + Count;
	Count++;
}
