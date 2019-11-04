#pragma once

class RHICommandList;
class Material;
class MeshDrawCommand
{
public:
	MeshDrawCommand();
	~MeshDrawCommand();
	RHIBuffer* Vertex;
	RHIBuffer* Index;
	RHIBuffer* TransformUniformBuffer;
	RHIBuffer* MaterialInstanceBuffer = nullptr;
	uint64 FirstIndex = 0;
	uint64 NumPrimitves = 0;
	uint64 NumInstances = 0;
	Material* TargetMaterial = nullptr;
	void SubmitDraw(RHICommandList* Cmd);
	GameObject* Object = nullptr;
};

