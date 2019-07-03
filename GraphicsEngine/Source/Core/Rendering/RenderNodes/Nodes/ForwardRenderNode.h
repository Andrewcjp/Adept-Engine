#pragma once
#include "../RenderNode.h"
class ForwardRenderNode : public RenderNode
{
public:
	ForwardRenderNode();
	~ForwardRenderNode();

	virtual void OnExecute() override;
	virtual void OnSetupNode() override;
	virtual std::string GetName() const override;
	bool UseLightCulling = false;
	bool UsePreZPass = false;
	bool EnableShadows = false;
private:
	RHICommandList* CommandList = nullptr;
protected:
	virtual void OnNodeSettingChange() override;

};

