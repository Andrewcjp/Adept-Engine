#include "DebugHMD.h"
#include "VRCamera.h"
#include "RHI/RHI.h"
#include "HMDManager.h"

DebugHMD::DebugHMD()
{}

DebugHMD::~DebugHMD()
{}

void DebugHMD::Update()
{
	CameraInstance->UpdateDebugTracking(RHI::GetVrSettings()->EyeDistance);
}

bool DebugHMD::IsActive()
{
	return true;
}
