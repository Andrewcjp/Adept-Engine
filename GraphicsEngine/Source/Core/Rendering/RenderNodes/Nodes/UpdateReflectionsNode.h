#pragma once
#include "..\RenderNode.h"
class UpdateReflectionsNode: public RenderNode
{
public:
	UpdateReflectionsNode();
	~UpdateReflectionsNode();

	virtual void OnExecute() override;


	virtual std::string GetName() const override;

protected:
	virtual void OnNodeSettingChange() override;


	virtual void OnSetupNode() override;
private:
	RHICommandList* CubemapCaptureList = nullptr;
	RHICommandList* ComputeList = nullptr;
};

