#pragma once
#include "RHI\ShaderBase.h"

class FrameBuffer;
class RHICommandList;
class Shader_Pair;
//this handles everything relating to software VRS and VRR
class VRXEngine
{
public:
	VRXEngine();
	~VRXEngine();
	static VRXEngine* Get();
	static void ResolveVRRFramebuffer_PS(RHICommandList * list, FrameBuffer * Target, RHITexture * ShadingImage);
	//resolve the Framebuffer to a single image.
	static void ResolveVRRFramebuffer(RHICommandList* list, FrameBuffer* Target, RHITexture* ShadingImage);

	void SetVRRShadingRate(RHICommandList* List, int FactorIndex);
	void SetVRXShadingRateImage(RHICommandList* List, RHITexture* Target);
	static void SetupVRRShader(Shader* S, DeviceContext* device);
	static void AddVRRToRS(std::vector<ShaderParameter>& S, int lastindex = 0);
	static void WriteVRRStencil(RHICommandList* List,FrameBuffer * MainBuffer);
private:
	Shader_Pair* ResolvePS = nullptr;
	Shader_Pair* StencilWriteShader = nullptr;
	Shader_Pair* VRRClassifyShader = nullptr;
	Shader_Pair* VRRLaunchShader = nullptr;
	RHIBuffer* VARTileList = nullptr;
	RHIBuffer* TileData = nullptr;
	RHIBuffer* IndirectCommandBuffer;
	static VRXEngine* Instance;
};

