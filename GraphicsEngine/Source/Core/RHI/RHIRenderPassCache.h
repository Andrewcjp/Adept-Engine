#pragma once
class RHIRenderPassCache
{
public:
	RHIRenderPassCache();
	~RHIRenderPassCache();
	RHI_API RHIRenderPass* GetOrCreatePass(RHIRenderPassDesc & desc);
	RHI_API static RHIRenderPassCache* Get();
private:
	std::vector<RHIRenderPass*> Passes;
};

