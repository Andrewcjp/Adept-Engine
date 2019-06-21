#pragma once
#include "../StorageNode.h"
class SceneDataNode : public StorageNode
{
public:
	SceneDataNode();
	~SceneDataNode();
	Scene* CurrnetScene = nullptr;

	virtual void Update() override;

protected:
	virtual void Create() override;

};

