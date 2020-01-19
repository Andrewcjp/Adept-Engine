#pragma once
#include "Rendering/RenderNodes/RenderGraphSystem.h"

class RenderGraph;
class RenderGraphPatchLibrary
{
public:
	RenderGraphPatchLibrary();
	~RenderGraphPatchLibrary();
	typedef std::function<bool(RenderGraph*)> PatchFunc;
	bool ApplyPatch(EBuiltInRenderGraphPatch::Type Type, RenderGraph* graph);
	void RegisterPatchFunction(EBuiltInRenderGraphPatch::Type Type, PatchFunc Func);

	void OnInit();

	bool AddVRX(RenderGraph* Graph);
	bool AddRT_Reflections(RenderGraph * Graph);
	bool AddVoxelReflections(RenderGraph * Graph);
	bool AddVoxel_RTReflections(RenderGraph * Graph);
private:
	std::vector<PatchFunc> Funcs;
};
namespace ERG_Patch_Reflections
{
	enum Type
	{
		In_GBuffer,
		In_ShadowData,
		In_Limit,
		Out_LightNode,
		Limit,
	};
};

