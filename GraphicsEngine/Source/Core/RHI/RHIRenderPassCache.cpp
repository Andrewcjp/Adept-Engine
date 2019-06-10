#include "RHIRenderPassCache.h"


RHIRenderPassCache::RHIRenderPassCache()
{}


RHIRenderPassCache::~RHIRenderPassCache()
{}

RHIRenderPass * RHIRenderPassCache::GetOrCreatePass(RHIRenderPassDesc & desc)
{
	desc.Build();
	for (int i = 0; i < Passes.size(); i++)
	{
		if (Passes[i]->Desc == desc)
		{
			return Passes[i];
		}
	}
	RHIRenderPass* RP = RHI::GetRHIClass()->CreateRenderPass(desc, RHI::GetDefaultDevice());
	RP->Complie();
	Passes.push_back(RP);
	if (RHI::GetFrameCount() > 0)
	{
		Log::LogMessage("Creating RenderPass at runtime", Log::Warning);
	}
	return RP;
}

RHIRenderPassCache * RHIRenderPassCache::Get()
{
	return RHI::Get()->RenderPassCache;
}
