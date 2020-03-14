#include "TerrainRenderer.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Mesh/MeshBatch.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "RHI/RHIBufferGroup.h"

REGISTER_SHADER_VS(Terrain_Vs, "Terrain\\Terrain_Vs");
REGISTER_SHADER_PS(Terrain_Ps, "Terrain\\Terrain_PS_Def");

TerrainRenderer* TerrainRenderer::Instance = nullptr;
TerrainRenderer::TerrainRenderer()
{
	TMP = new GameObject();
	TMP->GetTransform()->SetPos(glm::vec3(20, 10,0));
	const int size = 10;
	TMP->GetTransform()->SetScale(glm::vec3(size, 1, size));
	QuadMesh = RHI::CreateMesh("models\\TerrainQuad.obj");
	QuadMesh->PrepareDataForRender(TMP);
	TerrainShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Terrain\\Terrain_Vs","Terrain\\Terrain_PS_Def" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT });
	HeightMap = AssetManager::DirectLoadTextureAsset("Terrain\\Maps\\HeightMap_pack.png");
	BaseTex = AssetManager::DirectLoadTextureAsset("Terrain\\Maps\\BackedTexture.png");
}

TerrainRenderer::~TerrainRenderer()
{}

void TerrainRenderer::Update()
{}

void TerrainRenderer::RenderTerrainForDepth(FrameBuffer * Buffer, RHICommandList* list)
{}

void TerrainRenderer::RenderTerrainGBuffer(FrameBuffer * Buffer, RHICommandList* list, FrameBuffer * DepthSource)
{
	RHIPipeLineStateDesc State = RHIPipeLineStateDesc::CreateDefault(TerrainShader, Buffer);
	State.RenderTargetDesc = Buffer->GetPiplineRenderDesc();
	if (DepthSource != nullptr)
	{
		State.RenderTargetDesc = Buffer->GetPiplineRenderDesc();
		State.RenderTargetDesc.DSVFormat = DepthSource->GetPiplineRenderDesc().DSVFormat;
	}
	list->SetPipelineStateDesc(State);
	list->SetTexture(HeightMap, "HeightMap");
	list->SetTexture(BaseTex, "BaseTex");
	SceneRenderer::Get()->BindMvBuffer(list);
	RHIRenderPassDesc info(Buffer, ERenderPassLoadOp::Load);
	info.DepthSourceBuffer = DepthSource;
	list->BeginRenderPass(info);
	RenderQuad(list);
	list->EndRenderPass();
}

TerrainRenderer * TerrainRenderer::Get()
{
	if (Instance == nullptr)
	{
		Instance = new TerrainRenderer();
	}
	return Instance;
}

void TerrainRenderer::RenderQuad(RHICommandList * List)
{
	MeshBatch* batch = QuadMesh->GetMeshBatch();
	if (batch->elements.size() > 0)
	{
		List->SetConstantBufferView(batch->elements[0]->TransformBuffer->Get(List), 0, "GOConstantBuffer");
		List->SetVertexBuffer(batch->elements[0]->VertexBuffer->Get(List));
		List->SetIndexBuffer(batch->elements[0]->IndexBuffer->Get(List));
		List->DrawIndexedPrimitive(batch->elements[0]->NumPrimitives, 1, 0, 0, 0);
	}
}

