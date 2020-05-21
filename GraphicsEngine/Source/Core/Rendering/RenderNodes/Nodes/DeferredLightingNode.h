#pragma once
#include "../RenderNode.h"

class Shader_Deferred;
class Shader_Pair;

class DeferredLightingNode : public RenderNode
{
public:
	DeferredLightingNode();
	virtual ~DeferredLightingNode();
	virtual void OnExecute() override;

	
	

	virtual void OnNodeSettingChange() override;
	NameNode("Deferred Lighting");
	bool UseScreenSpaceReflection = false;
	InputData(DeferredLightingInputs,
			NodeLink * GBuffer = nullptr;
			NodeLink* MainBuffer = nullptr;
			NodeLink* SceneData = nullptr;
			NodeLink* ShadowMask = nullptr;
			NodeLink* ShadowMaps = nullptr;
			NodeLink* SSRData = nullptr;
			);
	void AddSkyBoxToGraph(RenderGraph* graph, RenderNode* Node, DeferredLightingInputs* Inputs);
	struct SkyBoxData
	{
		NodeLink * GBuffer = nullptr;
		NodeLink* MainBuffer = nullptr;
	};
	SkyBoxData m_SkyBoxData;
	void ExecuteSkybox(SkyBoxData & data, RHICommandList * list);
private:
	Shader_Deferred* DeferredShader = nullptr;
};

