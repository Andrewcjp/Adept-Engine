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
	int FirstIndex = 0;
	int NumPrimitves = 0;
	int NumInstances = 0;
	Material* TargetMaterial = nullptr;
	void SubmitDraw(RHICommandList* Cmd);
	GameObject* Object = nullptr;
};

