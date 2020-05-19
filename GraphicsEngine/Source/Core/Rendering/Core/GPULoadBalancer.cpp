#include "GPULoadBalancer.h"
#include "RHI/SFRController.h"

GPULoadBalancer* GPULoadBalancer::Instance = nullptr;

GPULoadBalancer::GPULoadBalancer()
{
	Instance = this;
	SFR_Controller = new SFRController();
}

GPULoadBalancer::~GPULoadBalancer()
{}

void GPULoadBalancer::Tick()
{
	if (SFR_Controller != nullptr)
	{
		SFR_Controller->Tick();
	}
}
