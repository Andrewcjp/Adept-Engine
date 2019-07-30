#pragma once

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
	void ResolveVRRFramebuffer(RHICommandList* list, FrameBuffer* Target);
	//resolve the Framebuffer to a single image.
	void ResolveVRSFramebuffer(RHICommandList* list, FrameBuffer* Target);

	void SetVRSShadingRate(RHICommandList* List, VRS_SHADING_RATE::type Rate);
	void SetVRRShadingRate(RHICommandList* List, int FactorIndex);
	void SetVRXShadingRateImage(RHICommandList* List, FrameBuffer* Target);

private:

};

