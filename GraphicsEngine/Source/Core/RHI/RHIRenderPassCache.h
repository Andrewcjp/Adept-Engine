#pragma once
class RHIRenderPassCache
{
public:
	RHIRenderPassCache();
	~RHIRenderPassCache();
	RHI_API RHIRenderPass* GetOrCreatePass(const RHIRenderPassDesc & desc);
	RHI_API static RHIRenderPassCache* Get();
private:
	std::vector<RHIRenderPass*> Passes;
};

