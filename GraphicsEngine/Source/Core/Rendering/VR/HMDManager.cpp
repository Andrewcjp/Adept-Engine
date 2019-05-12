#include "Stdafx.h"
#include "HMDManager.h"
#include "HMD.h"
#include "Core\BaseWindow.h"


HMDManager::HMDManager()
{}


HMDManager::~HMDManager()
{}

void HMDManager::Init()
{
	return;
	HeadSet = HMD::Create();
	if (HeadSet != nullptr)
	{
		HeadSet->Init();
		Log::LogMessage("Found VR HMD");
	}
	HeadSet->UpdateProjection(2.5);
	RHI::GetRenderSettings()->LockedWidth = HeadSet->GetDimentions().x;
	RHI::GetRenderSettings()->LockedHeight = HeadSet->GetDimentions().y;
	//BaseWindow::StaticResize();
}

HMD * HMDManager::GetHMD()
{
	return HeadSet;
}

void HMDManager::Update()
{
	if (HeadSet != nullptr)
	{
		HeadSet->Update();
	}
}

