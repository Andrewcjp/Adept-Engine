#include "SamplerFeedbackEngine.h"
#include "RHI/RHITexture.h"
#include "Core/GameObject.h"
#include "Rendering/Core/Mesh/MeshBatch.h"
#include "RHI/RHIBufferGroup.h"
#include "Rendering/Shaders/GlobalShaderLibrary.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Core/Defaults.h"
#include "Rendering/Shaders/CrossDeviceShaderPair.h"
SamplerFeedbackEngine* SamplerFeedbackEngine::Instance = nullptr;

void SamplerFeedbackEngine::SetupPairedTexture(RHITexture * Target, RHIBuffer* ReadbackResouce)
{
	Target->PairedTexture = RHI::GetRHIClass()->CreateTexture2();
	RHITextureDesc2 desc = Target->GetDescription();
	desc.AllowUnorderedAccess = true;
	const int TileSize = RHI::GetRenderSettings()->GetSFSSettings().TileSize;
	desc.Width = desc.Width / TileSize;
	desc.Height = desc.Height / TileSize;
	desc.Format = eTEXTURE_FORMAT::FORMAT_R32_FLOAT;
	desc.AllowCrossGPU = false;
	desc.MipCount = 1;
	Target->PairedTexture->Create(desc);
	Instance->GetMipChainTexture(Target->GetDescription().Width, Target->GetDescription().MipCount);
	if (ReadbackResouce != nullptr)
	{
		desc.CPUVisible = true;
		RHIBufferDesc bufferdesc;
		bufferdesc.UseForReadBack = true;
		bufferdesc.ElementCount = desc.Width*desc.Height;
		bufferdesc.Stride = 4;
		bufferdesc.Accesstype = EBufferAccessType::ReadBack;
		ReadbackResouce->CreateBuffer(bufferdesc);
	}
}

RHITexture * SamplerFeedbackEngine::AccessParedTexture(RHITexture * target)
{
	return target->PairedTexture;
}

void SamplerFeedbackEngine::RenderTest(RHICommandList* list, FrameBuffer* Target)
{
	if (RHI::GetFrameCount() < 10)
	{
		return;
	}
	RHIPipeLineStateDesc State = RHIPipeLineStateDesc::CreateDefault(GlobalShaderLibrary::TestSamplerFeedbackShader->Get(list->GetDevice()), Target);
	State.InputLayout.Elements = Shader_Main::GetMainVertexFormat();
	State.DepthStencilState.DepthEnable = true;
	list->SetPipelineStateDesc(State);


	list->SetTexture2(BaseTex, "Diffuse");
	//list->SetTexture2(Defaults::GetDefaultTexture2(), "Diffuse");
	list->SetUAV(BaseTex->PairedTexture, "g_feedback");
	BaseTex->PairedTexture->SetState(list, EResourceState::UAV);
	list->ClearUAVFloat(BaseTex->PairedTexture, glm::vec4(20));
	RHITexture* MiPChain = GetMipChainTexture(BaseTex->GetDescription().Width, BaseTex->GetDescription().MipCount);
	list->SetTexture2(MiPChain, "SFS_MipTexture");
	MiPChain->SetState(list, EResourceState::PixelShader);
	SceneRenderer::Get()->BindMvBuffer(list);
	RHIRenderPassDesc info(Target, ERenderPassLoadOp::Load);
	//info.DepthSourceBuffer = DepthSource;
	list->BeginRenderPass(info);
	RenderQuad(list);
	list->EndRenderPass();
	list->UAVBarrier(BaseTex->PairedTexture);
	ResolveAndReadback(list, BaseTex->PairedTexture, OutputData);
	float* Data = (float*)OutputData->MapReadBack();
	OutputData->UnMap();
}

SamplerFeedbackEngine::SamplerFeedbackEngine()
{


}
void SamplerFeedbackEngine::Init()
{
	TMP = new GameObject();
	TMP->GetTransform()->SetPos(glm::vec3(20, 5, 0));
	const int size = 1;
	TMP->GetTransform()->SetScale(glm::vec3(size, 1, size));
	QuadMesh = RHI::CreateMesh("models\\Plane.obj");
	QuadMesh->PrepareDataForRender(TMP);
	AssetManager::DirectLoadTextureAsset("Terrain\\Maps\\BackedTexture.png");
	AssetPathRef p = AssetPathRef("BackedTexture.dds");
	p.IsDDC = true;
	p.DDCPath = "\\DerivedDataCache\\BackedTexture.dds";
	BaseTex = BaseTexture::CreateFromFile2(p);//AssetManager::DirectLoadTextureAsset("Terrain\\Maps\\BackedTexture.png");
	//BaseTex->
	OutputData = RHI::GetRHIClass()->CreateRHIBuffer(ERHIBufferType::ReadBack);
	SetupPairedTexture(BaseTex, OutputData);


}
void SamplerFeedbackEngine::RenderQuad(RHICommandList * List)
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
RHITexture * SamplerFeedbackEngine::GetMipChainTexture(int size, int mipcount)
{
	auto itor = MipChainTextures.find(size);
	if (itor != MipChainTextures.end())
	{
		return itor->second;
	}
	RHITexture *TestMipText = RHI::GetRHIClass()->CreateTexture2();
	CreateMipCheckingTexture(TestMipText, size, mipcount);
	MipChainTextures.emplace(size, TestMipText);
	return TestMipText;
}

void SamplerFeedbackEngine::ResolveAndReadback(RHICommandList* list, RHITexture * FeedbackMap, RHIBuffer * CPUData)
{
	list->CopyResource(FeedbackMap, CPUData);
}

#define TEX_TYPE float
void SamplerFeedbackEngine::CreateMipCheckingTexture(RHITexture* Texture, int Size, int Mipcount)
{
	TextureDescription Desc;
	Desc.Width = Size;
	Desc.Height = Size;
	Desc.MipLevels = Mipcount;
	Desc.Format = eTEXTURE_FORMAT::FORMAT_R32_FLOAT;
	Desc.Faces = 1;
	Desc.BitDepth = sizeof(float);
	int PixelCount = 0;
	int CSize = Size;
	for (int i = 0; i < Mipcount; i++)
	{
		PixelCount += CSize * CSize;
		Desc.MipLevelExtents.push_back(glm::ivec2(CSize, CSize));
		CSize /= 2;
	}

	TEX_TYPE* TextureData = new TEX_TYPE[PixelCount];
	memset(TextureData, 0, PixelCount * sizeof(TEX_TYPE));
	TEX_TYPE* Ptr = TextureData;
	for (int i = 0; i < Mipcount; i++)
	{
		int MipSize = Desc.MipExtents(i).x*Desc.MipExtents(i).x;
		for (int x = 0; x < MipSize; x++)
		{
			Ptr[x] = 1 + i;// (float)(Mipcount - i);
		}
		Ptr += MipSize;
	}
	Desc.ImageByteSize = sizeof(TEX_TYPE)*PixelCount;
	Desc.PtrToData = TextureData;
	Texture->CreateWithUpload(Desc, RHI::GetDefaultDevice());
}