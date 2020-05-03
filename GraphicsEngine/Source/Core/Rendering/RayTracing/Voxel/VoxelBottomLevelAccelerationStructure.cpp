#include "VoxelBottomLevelAccelerationStructure.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Rendering/Core/Material.h"
#include "Rendering/Core/Mesh/MeshBatch.h"
#include "Rendering/Core/SceneRenderer.h"
#include "RHI/RHIBufferGroup.h"
#include "RHI/RHITexture.h"
#include "RHI/Streaming/TextureStreamingCommon.h"
#include "VoxelTracingEngine.h"


VoxelBottomLevelAccelerationStructure::VoxelBottomLevelAccelerationStructure()
{
}


VoxelBottomLevelAccelerationStructure::~VoxelBottomLevelAccelerationStructure()
{
}

void VoxelBottomLevelAccelerationStructure::Init(MeshRendererComponent * object)
{
	CullingAABB* bounds = object->GetMesh()->GetBounds();
	MapSize = glm::ivec3((bounds->GetHalfExtends() * 2.1) / VoxelTracingEngine::Get()->ControlData.VoxelSize) + glm::ivec3(1, 1, 1);
	MapSize = glm::max(MapSize, glm::ivec3(1, 1, 1));
	const float MaxBound = 1500;
	MapSize = glm::min(MapSize, glm::ivec3(MaxBound, MaxBound, MaxBound));
	VoxelBuffer = RHI::GetRHIClass()->CreateTexture2();
	RHITextureDesc2 TextureDesc;
	TextureDesc.Width = GetMapSize().x;
	TextureDesc.Height = GetMapSize().y;
	TextureDesc.Depth = GetMapSize().z;
	TextureDesc.clearcolour = glm::vec4(0, 0, 0, 0);
	TextureDesc.AllowUnorderedAccess = true;
	TextureDesc.Dimension = DIMENSION_TEXTURE3D;
	TextureDesc.Format = FORMAT_R32G32B32A32_UINT;
	TextureDesc.InitalState = EResourceState::UAV;
	GetVoxelBuffer()->Create(TextureDesc, Device);

	TargetMesh = object;
	PrimitiveTransfromBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	PrimitiveTransfromBuffer->CreateConstantBuffer(sizeof(MeshTransfromBuffer), 1, true);
	MeshTransfromBuffer b;
	glm::mat4 identity = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	b.M = identity;
	PrimitiveTransfromBuffer->UpdateConstantBuffer(&b);

	VxControlData = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	VxControlData->CreateConstantBuffer(sizeof(VXData), 1, true);
	ControlData = VoxelTracingEngine::Get()->ControlData;
	ControlData.VoxelRes = MapSize;
	ControlData.Update();
	VxControlData->UpdateConstantBuffer(&ControlData);

}

void VoxelBottomLevelAccelerationStructure::Build(RHICommandList* list)
{
	BuildState = true;
	GetVoxelBuffer()->SetState(list, EResourceState::UAV);
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(VoxelTracingEngine::Get()->voxeliseShader);
	desc.DepthStencilState.DepthEnable = false;
	desc.RasterizerState.ConservativeRaster = true;
	desc.RasterizerState.Cull = false;
	desc.Cull = false;
	list->SetPipelineStateDesc(desc);
	list->SetViewport(0, 0, GetMapSize().x, GetMapSize().y, 0, 1.0f);
	list->SetUAV(GetVoxelBuffer(), "voxelTex");
	list->SetConstantBufferView(VxControlData, 0, "VoxelDataBuffer");
	MeshBatch* Batch = TargetMesh->GetMesh()->GetMeshBatch();
	if (TargetMesh->GetMaterial(0)->TestHandle != nullptr)
	{
		TargetMesh->GetMaterial(0)->TestHandle->Bind(list, "BaseTex");
	}
	for (int i = 0; i < Batch->elements.size(); i++)
	{
		MeshBatchElement* Element = Batch->elements[i];
		list->SetConstantBufferView(PrimitiveTransfromBuffer, 0, "GOConstantBuffer");
		list->SetVertexBuffer(Element->VertexBuffer->Get(list));
		list->SetIndexBuffer(Element->IndexBuffer->Get(list));
		list->DrawIndexedPrimitive(Element->NumPrimitives, Element->NumInstances, 0, 0, 0);
	}

	GetVoxelBuffer()->SetState(list, EResourceState::Non_PixelShader);
}

void VoxelBottomLevelAccelerationStructureInstance::SetTransForm(GameObject * GO)
{
	LinkedObject = GO;
}
