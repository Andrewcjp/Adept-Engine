#pragma once
#include "../RenderNode.h"
//handles the Standard window UI etc.
class UINode : public RenderNode
{
public:
	UINode();
	~UINode();

	void OnExecute() override;

protected:
	void OnNodeSettingChange() override;

};

