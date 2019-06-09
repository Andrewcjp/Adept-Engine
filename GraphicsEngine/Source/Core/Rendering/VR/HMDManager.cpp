#include "HMDManager.h"


HMDManager::HMDManager()
{}


HMDManager::~HMDManager()
{}

void HMDManager::Init()
{	
	HeadSet = HMD::Create();
	if (HeadSet != nullptr)
	{
		HeadSet->Init();
		RHI::GetRenderSettings()->LockBackBuffer = true;
		RHI::GetRenderSettings()->LockedWidth = HeadSet->GetDimentions().x;
		RHI::GetRenderSettings()->LockedHeight = HeadSet->GetDimentions().y;
	}
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

