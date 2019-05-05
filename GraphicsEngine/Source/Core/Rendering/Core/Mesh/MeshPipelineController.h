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
	void RenderPass(ERenderPass::Type type, RHICommandList* List, Shader* shader = nullptr);

	void Init();

	Scene* TargetScene = nullptr;
private:
	std::vector<MeshBatch*> Batches;
	MeshBatchProcessor* Processors[ERenderPass::Limit];
};

