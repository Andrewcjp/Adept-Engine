#pragma once
#include "RHI\ShaderBase.h"

class FrameBuffer;
class RHICommandList;
//this handles everything relating to software VRS and VRR
class VRXEngine
{
public:
	VRXEngine();
	~VRXEngine();
	static VRXEngine* Get();
	//resolve the Framebuffer to a single image.
	static void ResolveVRRFramebuffer(RHICommandList* list, FrameBuffer* Target, FrameBuffer* ShadingImage);
	//resolve the Framebuffer to a single image.
	void ResolveVRSFramebuffer(RHICommandList* list, FrameBuffer* Target);

	void SetVRSShadingRate(RHICommandList* List, VRS_SHADING_RATE::type Rate);
	void SetVRRShadingRate(RHICommandList* List, int FactorIndex);
	void SetVRXShadingRateImage(RHICommandList* List, FrameBuffer* Target);
	static void SetupVRRShader(Shader* S);
	static void AddVRRToRS(std::vector<ShaderParameter>& S, int lastindex = 0);
	static void SetupVRSShader(Shader* S);
	static void AddVRSToRS(std::vector<ShaderParameter>& S, uint64 lastindex = 0);
private:

};

