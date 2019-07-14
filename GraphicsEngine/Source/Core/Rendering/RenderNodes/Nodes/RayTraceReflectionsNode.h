#pragma once
#include "..\RenderNode.h"
class RayTraceReflectionsNode: public RenderNode
{
public:
	RayTraceReflectionsNode();
	~RayTraceReflectionsNode();

	virtual void OnExecute() override;
	virtual bool IsNodeSupported(const RenderSettings& settings) override;
protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;

};

