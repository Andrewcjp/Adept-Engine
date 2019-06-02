#pragma once
class RHIRenderPassCache
{
public:
	RHIRenderPassCache();
	~RHIRenderPassCache();
	RHIRenderPass* GetOrCreatePass(RHIRenderPassDesc & desc);
private:
	std::vector<RHIRenderPass*> Passes;
};

