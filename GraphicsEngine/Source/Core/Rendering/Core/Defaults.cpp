#include "Defaults.h"
#include "Core\Assets\AssetManager.h"
#include "Core\Platform\PlatformCore.h"
#include "Core\IRefCount.h"
#include "Core\Assets\Asset types/Asset_Shader.h"
#include "Core\Assets\ShaderCompiler.h"
#include "RHI\RHICommandList.h"

Defaults* Defaults::Instance = nullptr;

Defaults::Defaults()
{
	DefaultTexture = AssetManager::DirectLoadTextureAsset("\\texture\\T_GridSmall_01_D.png");
	ensureFatalMsgf(DefaultTexture != nullptr, "Failed to Load Fallback Texture");
	AssetPathRef r = AssetPathRef("\\texture\\T_GridSmall_01_D.png");
	r.IsDDC = true;
	r.DDCPath =  "\\DerivedDataCache\\T_GridSmall_01_D.dds";
	DefaultRHITex = BaseTexture::CreateFromFile2(r);

	DefaultMateral = new Material(new Asset_Shader(true));
	DefaultMateral->Init();
	DefaultMateral->SetDiffusetexture(DefaultTexture);
	float g_quad_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,0.0f,
	1.0f, -1.0f, 0.0f,0.0f,
	-1.0f,  1.0f, 0.0f,0.0f,
	-1.0f,  1.0f, 0.0f,0.0f,
	1.0f, -1.0f, 0.0f,0.0f,
	1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4, EBufferAccessType::Dynamic);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
	ShaderCompiler::Get()->TickMaterialCompile();
}

Defaults::~Defaults()
{
	//	SafeDelete(DefaultShaderMat);
}

void Defaults::Start()
{
	Instance = new Defaults();
}

void Defaults::Shutdown()
{
	SafeDelete(Instance);
}

BaseTextureRef Defaults::GetDefaultTexture()
{
	return Instance->DefaultTexture;
}

Material * Defaults::GetDefaultMaterial()
{
	return Instance->DefaultMateral;
}

RHITexture * Defaults::GetDefaultTexture2()
{
	return Instance->DefaultRHITex;
}


RHIBuffer * Defaults::GetQuadBuffer()
{
	return Instance->VertexBuffer;
}


