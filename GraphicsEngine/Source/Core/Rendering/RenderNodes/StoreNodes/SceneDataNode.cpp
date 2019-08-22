
#include "SceneDataNode.h"
#include "Core/BaseWindow.h"


SceneDataNode::SceneDataNode()
{
	StoreType = EStorageType::SceneData;
}


SceneDataNode::~SceneDataNode()
{}

void SceneDataNode::Update()
{
	CurrnetScene = BaseWindow::GetScene();
}

void SceneDataNode::Create()
{

}
