#pragma once
struct InstanceArgs
{
	glm::mat4x4 M;
	glm::vec4 data;
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
	RHIBuffer* GetMaterialBuffer();
	Material* TargetMaterial = nullptr;
	bool IsCompletelyCulled()const;
private:
	std::vector<MeshBatch*> containedBatches;
	RHIBuffer* Buffer = nullptr;
	RHIBuffer* MateralDataBuffer = nullptr;
	int Stride = 0;
	bool IsCulled = false;
};

