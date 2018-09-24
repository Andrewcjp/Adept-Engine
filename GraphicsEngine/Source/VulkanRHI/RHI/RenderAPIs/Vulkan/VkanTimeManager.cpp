#include "VkanTimeManager.h"



VkanTimeManager::VkanTimeManager():RHITimeManager(nullptr)
{}


VkanTimeManager::~VkanTimeManager()
{}

void VkanTimeManager::UpdateTimers()
{}

std::string VkanTimeManager::GetTimerData()
{
	return std::string();
}

void VkanTimeManager::SetTimerName(int index, std::string Name)
{}

void VkanTimeManager::StartTotalGPUTimer(RHICommandList * ComandList)
{}

void VkanTimeManager::StartTimer(RHICommandList * ComandList, int index)
{}

void VkanTimeManager::EndTimer(RHICommandList * ComandList, int index)
{}

void VkanTimeManager::EndTotalGPUTimer(RHICommandList * ComandList)
{}

float VkanTimeManager::GetTotalTime()
{
	return 0.0f;
}

void VkanTimeManager::ResolveCopyTimeHeaps(RHICommandList * ComandList)
{}
