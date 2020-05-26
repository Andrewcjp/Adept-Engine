#pragma once
#include "..\RenderNode.h"

class NodeLink;
class StorageNode;
class LightCullingNode : public RenderNode
{
public:
	LightCullingNode();
	~LightCullingNode();

	virtual void OnExecute() override;
	virtual std::string GetName() const override;
	InputData(LightCullInput,
		NodeLink* CulledLightList = nullptr;
	NodeLink* DepthBuffer = nullptr; );
	void AddApplyToGraph(RenderGraph* Graph,  StorageNode* gBuffer, StorageNode* ShadowMask, StorageNode* MainBuffer);
	struct ApplyPassData
	{
		NodeLink* TileList;
		NodeLink* LightData;
		NodeLink* GBuffer;
		NodeLink* ShadowMask;
		NodeLink* MainBuffer;
	};
	void ExecuteApply(ApplyPassData& Data, RHICommandList* list);
protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;
	ApplyPassData Data;
};

