#include "VKNTimeManager.h"



VKNTimeManager::VKNTimeManager():RHITimeManager(nullptr)
{}


VKNTimeManager::~VKNTimeManager()
{}

void VKNTimeManager::UpdateTimers()
{}



void VKNTimeManager::SetTimerName(int index, std::string Name)
{}

void VKNTimeManager::SetTimerName(int index, std::string Name, ECommandListType::Type type)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKNTimeManager::StartTotalGPUTimer(RHICommandList * ComandList)
{}

void VKNTimeManager::StartTimer(RHICommandList * ComandList, int index)
{}

void VKNTimeManager::EndTimer(RHICommandList * ComandList, int index)
{}

void VKNTimeManager::EndTotalGPUTimer(RHICommandList * ComandList)
{}

float VKNTimeManager::GetTotalTime()
{
	return 0.0f;
}

