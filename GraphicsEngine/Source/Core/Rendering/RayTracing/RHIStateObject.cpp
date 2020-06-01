#include "RHIStateObject.h"


RHIStateObject::RHIStateObject(DeviceContext* D,RHIStateObjectDesc desc)
{
	Device = D;
	Desc = desc;
	for (int i = 0; i < desc.ShaderRecords.HitGroups.size(); i++)
	{
		ShaderHitGroups.emplace(desc.ShaderRecords.HitGroups[i]->Name, desc.ShaderRecords.HitGroups[i]);
	}
}

RHIStateObject::~RHIStateObject()
{}


ShaderHitGroupInstance* RHIStateObject::AllocateHitGroupInstance(std::string sourceName)
{
	auto itor = ShaderHitGroups.find(sourceName);
	if (itor != ShaderHitGroups.end())
	{
		return new ShaderHitGroupInstance(itor->second);
	}
	AD_ERROR("Missing Shader in state object '%s'", sourceName.c_str());
	return nullptr;
}