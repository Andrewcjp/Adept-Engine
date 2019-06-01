#pragma once
struct InstanceArgs
{
	glm::mat4x4 M;
};
class MeshInstanceBuffer
{
public:
	MeshInstanceBuffer();
	~MeshInstanceBuffer();
	void AddBatch(MeshBatch* Batch);
	void RemoveBatch(MeshBatch* batch);
	void UpdateBuffer();
	void Build();
	int GetInstanceCount();
	RHIBuffer* GetBuffer();
private:
	std::vector<MeshBatch*> containedBatches;
	RHIBuffer* Buffer = nullptr;
	int Stride = 0;
};

