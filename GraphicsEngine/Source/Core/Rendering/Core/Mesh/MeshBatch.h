#pragma once
#include "Core/Types/WeakObjectPtr.h"

class Material;
class MeshInstanceBuffer;
class RHIBufferGroup;
struct MeshBatchElement
{
	RHIBufferGroup* VertexBuffer = nullptr;
	RHIBufferGroup* IndexBuffer = nullptr;
	RHIBufferGroup* TransformBuffer = nullptr;
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
	WeakObjectPtr<GameObject> Owner = nullptr;
	void Update();
	bool NeedRecreate();
	//Are we instanced?
	bool IsinstancedBatch = false;
	//if this is null this is the instance control node.
	//else we are subnode.
	MeshBatch* InstanceOwner = nullptr;

	MeshInstanceBuffer* InstanceBuffer = nullptr;
	bool IsValid = true;
};

