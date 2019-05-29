#include "DepthOnlyMeshProcessor.h"
#include "Core\Assets\ShaderComplier.h"
#include "RHI\RHICommandList.h"
#include "MeshDrawCommand.h"
#include "..\SceneRenderer.h"
#include "MeshBatch.h"


DepthOnlyMeshProcessor::DepthOnlyMeshProcessor()
{
	Init();
	PassType = ERenderPass::DepthOnly;
}

DepthOnlyMeshProcessor::~DepthOnlyMeshProcessor()
{}

void DepthOnlyMeshProcessor::Init()
{

}


void DepthOnlyMeshProcessor::AddBatch(MeshBatch* Batch)
{
	Batch->Update();
	if (Batch->CastShadow && !Batch->ShadowPassCulled)
	{
		Process(Batch);
	}
}

void DepthOnlyMeshProcessor::Process(MeshBatch* Batch)
{
	for (int i = 0; i < Batch->elements.size(); i++)
	{
		MeshDrawCommand* command = new MeshDrawCommand();
		command->NumInstances = 1;
		command->NumPrimitves = Batch->elements[i]->NumPrimitives;
		command->Index = Batch->elements[i]->IndexBuffer;
		command->Vertex = Batch->elements[i]->VertexBuffer;
		command->TransformUniformBuffer = Batch->elements[i]->TransformBuffer;
		AddDrawCommand(command);
	}
}

void DepthOnlyMeshProcessor::SubmitCommands(RHICommandList* List, Shader* shader)
{
	for (int i = 0; i < DrawCommands.size(); i++)
	{
		MeshDrawCommand* C = DrawCommands[i];
		List->SetConstantBufferView(C->TransformUniformBuffer, 0, 0);
		List->SetVertexBuffer(C->Vertex);
		List->SetIndexBuffer(C->Index);
		List->DrawIndexedPrimitive(C->NumPrimitves, C->NumInstances, 0, 0, 0);
		CountDrawCall();
	}	
}

