#pragma once
class D3D12Query;
class D3D12DeviceContext;
class D3D12CommandList;
struct DX12QueryBatch
{
	int Startindex;
	int Count;
	std::vector<D3D12Query*> Queries;
	void Reset()
	{
		Startindex = 0;
		Count = 0;
		MemoryUtils::DeleteVector(Queries);
		Queries.clear();
	}
	void Alloc(D3D12Query* Q);
	bool Open = false;
};

class D3D12QueryHeap
{
public:
	D3D12QueryHeap(D3D12DeviceContext* device, int MaxSize, D3D12_QUERY_HEAP_TYPE type);
	~D3D12QueryHeap();
	void BeginQuery(D3D12CommandList* list, D3D12Query* Q);
	void EndQuerry(D3D12CommandList* list, D3D12Query* Q);
	void BeginQuerryBatch();
	void ResolveAndEndQueryBatches(D3D12CommandList* list);
	void ReadData();
private:
	void CreateHeap();
	void CreateResultsBuffer();
	DX12QueryBatch CurrentBatch;
	std::vector<DX12QueryBatch> CurrentActiveBatches;
	D3D12DeviceContext* Device = nullptr;
	int HeapSize = 0;
	D3D12_QUERY_HEAP_TYPE HeapType;
	ID3D12QueryHeap* GetHeap(int index = -1);
	ID3D12Resource* GetBuffer(int index = -1);
	D3D12_QUERY_TYPE GetType();
	struct QueryBuffers
	{
		ID3D12QueryHeap* QueryHeap = nullptr;
		ID3D12Resource* ResultBuffer = nullptr;
	};
	QueryBuffers Buffers[RHI::CPUFrameCount] = { 0 };
};
//Create batches of queries
//resolve them and dispose
//the underlying heap can be size at 8192 or something large like that
//the heap space will get reused.



//Alloc to querry
//REsolving will return timestamp 
//Time manager will need to stich togeatehr to update timers.