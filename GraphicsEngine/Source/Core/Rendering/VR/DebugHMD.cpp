#include "DebugHMD.h"
#include "VRCamera.h"

DebugHMD::DebugHMD()
{}

DebugHMD::~DebugHMD()
{}

void DebugHMD::Update()
{
	CameraInstance->UpdateDebugTracking();
}
