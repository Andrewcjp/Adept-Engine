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
	std::string ToString(ERenderPass::Type t);
}
//this is passed in to a render scene call to provided to all command submission code.
struct MeshPassRenderArgs
{
	bool UseDeferredShaders = false;
	ERenderPass::Type PassType = ERenderPass::Limit;
	bool UseShadows = false;
	bool ReadDepth = false;
	void* PassData = nullptr;
};
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
	void RemoveBatches(GameObject * owner);
	void ClearBatches();
	//attempt to merge the static objects in the scene
	void BuildStaticInstancing();

	void CreateInstanceController(MeshBatch* Ctl, std::map<RHIBuffer *, std::vector<MeshBatch *>>::iterator itor, int limit, int offset);

	//#todo: which one?
	TEMP_API void RenderPass(ERenderPass::Type type, RHICommandList* List, Shader* shader = nullptr, EBatchFilter::Type Filter = EBatchFilter::ALL);
	TEMP_API void RenderPass(const MeshPassRenderArgs & args, RHICommandList* List, Shader* shader = nullptr, EBatchFilter::Type Filter = EBatchFilter::ALL);
	void Init();

	Scene* TargetScene = nullptr;
private:
	std::vector<MeshBatch*> Batches;
	MeshBatchProcessor* Processors[ERenderPass::Limit];
};

