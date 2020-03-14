#pragma once

class RHICommandList;
class Material;
class RHIBufferGroup;
class MeshDrawCommand
{
public:
	MeshDrawCommand();
	~MeshDrawCommand();
	RHIBufferGroup* Vertex;
	RHIBufferGroup* Index;
	RHIBufferGroup* TransformUniformBuffer = nullptr;
	RHIBufferGroup* MaterialInstanceBuffer = nullptr;
	uint64 FirstIndex = 0;
	uint64 NumPrimitves = 0;
	uint64 NumInstances = 0;
	Material* TargetMaterial = nullptr;
	void SubmitDraw(RHICommandList* Cmd);
	GameObject* Object = nullptr;
};

