#pragma once
#include "Rendering/RenderNodes/RenderGraphSystem.h"
#include "RenderNode.h"

class RenderGraph;
class DeferredLightingNode;
class ShadowAtlasStorageNode;
class PostProcessNode;
class FrameBufferStorageNode;
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
struct RelfectionsPatchData
{
	FrameBufferStorageNode* GBuffer;
	ShadowAtlasStorageNode* ShadowData;
	FrameBufferStorageNode* VelocityBuffer;

	DeferredLightingNode*	DeferredLightNode = nullptr;
	RenderNode* UpdateStructuresNodeTarget = nullptr;
};

struct VRXPatchData
{	
	ShadowAtlasStorageNode* ShadowData;
	FrameBufferStorageNode* ShadowMask;

	DeferredLightingNode*	DeferredLightNode = nullptr;
	RenderNode* VizMode = nullptr;
	RenderNode* GBufferWrite = nullptr;
	PostProcessNode* PostNode = nullptr;
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

