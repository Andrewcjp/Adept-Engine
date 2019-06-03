#include "RHIRenderPassCache.h"


RHIRenderPassCache::RHIRenderPassCache()
{}


RHIRenderPassCache::~RHIRenderPassCache()
{}

RHIRenderPass * RHIRenderPassCache::GetOrCreatePass(RHIRenderPassDesc & desc)
{
	for (int i = 0; i < Passes.size(); i++)
	{
		if (Passes[i]->Desc == desc)
		{
			return Passes[i];
		}
	}
	RHIRenderPass* RP = RHI::GetRHIClass()->CreateRenderPass(desc, RHI::GetDefaultDevice());
	RP->Complie();
	return RP;
}

RHIRenderPassCache * RHIRenderPassCache::Get()
{
	return RHI::Get()->RenderPassCache;
}
