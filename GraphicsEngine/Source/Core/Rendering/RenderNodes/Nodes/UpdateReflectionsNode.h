#pragma once
#include "..\RenderNode.h"
class UpdateReflectionsNode: public RenderNode
{
public:
	UpdateReflectionsNode();
	~UpdateReflectionsNode();

	virtual void OnExecute() override;


	NameNode("Update Reflection Captures");

protected:
	virtual void OnNodeSettingChange() override;


	virtual void OnSetupNode() override;
private:
	RHICommandList* CubemapCaptureList = nullptr;
	RHICommandList* ComputeList = nullptr;
};

