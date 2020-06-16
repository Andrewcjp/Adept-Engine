#pragma once
#include "Core/Platform/PlatformCore.h"
#include "RHITypes.h"
#include "BaseTexture.h"

class DeviceContext;
struct RHIScissorRect;
typedef RHIScissorRect RHICopyRect;
//Represents a GPU resource 
struct RHITextureDesc2
{
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 Depth = 1;
	int MipCount = 1;
	bool IsRenderTarget = false;
	bool IsDepthStencil = false;
	bool AllowUnorderedAccess = false;
	bool AllowCrossGPU = false;
	bool CPUVisible = false;
	eTextureDimension Dimension = eTextureDimension::DIMENSION_TEXTURE2D;
	ETextureFormat Format = ETextureFormat::FORMAT_UNKNOWN;
	ETextureFormat RenderFormat = ETextureFormat::FORMAT_UNKNOWN;
	ETextureFormat DepthRenderFormat = ETextureFormat::FORMAT_UNKNOWN;
	glm::vec4 clearcolour = glm::vec4(0.0f, 0.2f, 0.4f, 1.0f);
	float DepthClearValue = 1.0f;
	std::string Name = "";

	EResourceState::Type InitalState = EResourceState::Common;
	RHI_API ETextureFormat GetRenderformat()const;
	RHI_API ETextureFormat GetDepthRenderformat() const;
};
class RHITexture : public IRHIResourse
{
public:
	RHI_API RHITexture();
	RHI_API virtual ~RHITexture();
	RHI_API const RHITextureDesc2& GetDescription()const;
	RHI_API virtual void Create(const RHITextureDesc2& Desc, DeviceContext* Context = nullptr);

	RHI_API virtual void CreateWithUpload(const TextureDescription & idesc, DeviceContext * iContext);

	void CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List);
	void CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* list);
	RHI_API virtual void CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List, const RHICopyRect & rect);
	RHI_API virtual void CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* list, const RHICopyRect & rect);
	RHI_API virtual void SetState(RHICommandList* list, EResourceState::Type State,int SubResource = -1) = 0;
	RHITexture* PairedTexture = nullptr;
	int LastClearFrame = 0;
protected:
	RHITextureDesc2 Desc;
	TextureDescription	UploadDesc;
	DeviceContext* Context = nullptr;
};

