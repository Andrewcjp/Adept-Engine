#pragma once

struct MeshBatchElement
{
	RHIBuffer* VertexBuffer = nullptr;
	RHIBuffer* IndexBuffer = nullptr;
	RHIBuffer* TransformBuffer = nullptr;
	int NumPrimitives = 0;
	int NumInstances = 0;
	Material* Material = nullptr;
	bool IsVisible = true;
};

class MeshBatch
{
public:
	MeshBatch();
	~MeshBatch();
	void AddMeshElement(MeshBatchElement* element);
	//private:
	std::vector<MeshBatchElement*> elements;
	bool CastShadow = true;
};


