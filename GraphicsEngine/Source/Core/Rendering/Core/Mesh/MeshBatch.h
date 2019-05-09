#pragma once

class Material;
struct MeshBatchElement
{
	RHIBuffer* VertexBuffer = nullptr;
	RHIBuffer* IndexBuffer = nullptr;
	RHIBuffer* TransformBuffer = nullptr;
	int NumPrimitives = 0;
	int NumInstances = 0;
	Material* MaterialInUse = nullptr;
	bool IsVisible = true;
	bool bTransparent = false;
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
	bool MainPassCulled = false;
	bool ShadowPassCulled = false;
};

