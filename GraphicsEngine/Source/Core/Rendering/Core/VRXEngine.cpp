#include "VRXEngine.h"


VRXEngine::VRXEngine()
{}


VRXEngine::~VRXEngine()
{}

VRXEngine * VRXEngine::Get()
{
	return nullptr;
}

void VRXEngine::ResolveVRRFramebuffer(RHICommandList* list, FrameBuffer* Target)
{
	//#VRX: todo
}

void VRXEngine::ResolveVRSFramebuffer(RHICommandList* list, FrameBuffer* Target)
{
	//#VRX: todo
}

void VRXEngine::SetVRSShadingRate(RHICommandList * List, VRS_SHADING_RATE::type Rate)
{
	//#VRX: todo
}

void VRXEngine::SetVRRShadingRate(RHICommandList * List, int FactorIndex)
{
	//#VRX: todo
}

void VRXEngine::SetVRXShadingRateImage(RHICommandList * List, FrameBuffer * Target)
{
	//#VRX: todo
}
