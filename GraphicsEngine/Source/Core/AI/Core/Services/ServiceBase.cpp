
#include "ServiceBase.h"


ServiceBase::ServiceBase()
{}

ServiceBase::~ServiceBase()
{}

void ServiceBase::HandleTick()
{
	Tick();
}

void ServiceBase::HandleInit()
{

}

void ServiceBase::Tick()
{

}

void ServiceBase::Init()
{

}

EBTNodeReturn::Type ServiceBase::GetServiceStatus()
{
	return CurrentStatus;
}
