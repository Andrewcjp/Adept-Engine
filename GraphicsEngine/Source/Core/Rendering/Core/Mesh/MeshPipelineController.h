#pragma once

class Scene;
class MeshBatchProcessor;
class SceneRenderer;
class MeshBatch;
namespace ERenderPass
{
	enum Type
	{
		DepthOnly,
		BasePass,
		BasePass_Cubemap,
		TransparentPass,
		PreZ,
		Limit
	};
}
namespace EBatchFilter
{
	enum Type
	{
		ALL,
		StaticOnly,
		DynamicOnly,
		Limit
	};
}
class MeshPipelineController
{
public:
	MeshPipelineController();
	~MeshPipelineController();
	void GatherBatches();
	//#todo: which one?
	void RenderPass(ERenderPass::Type type, RHICommandList* List, Shader* shader = nullptr, EBatchFilter::Type Filter = EBatchFilter::ALL);

	void Init();

	Scene* TargetScene = nullptr;
private:
	std::vector<MeshBatch*> Batches;
	MeshBatchProcessor* Processors[ERenderPass::Limit];
};

