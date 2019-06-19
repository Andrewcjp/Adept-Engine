#include "RenderGraph.h"
#include "Nodes/GBufferWriteNode.h"
#include "StorageNode.h"
#include "StorageNodeFormats.h"
#include "RenderNode.h"
#include "NodeLink.h"
#include "Nodes/DeferredLightingNode.h"

RenderGraph::RenderGraph()
{}

RenderGraph::~RenderGraph()
{}

void RenderGraph::RunGraph()
{
	RootNode->ExecuteNode();
}

void RenderGraph::CreateTestgraph()
{
	StorageNode* GBufferNode = AddStoreNode(new StorageNode());
	GBufferNode->StoreType = EStorageType::Framebuffer;
	GBufferNode->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new GBufferWriteNode();
	RootNode->GetInput(0)->SetStore(GBufferNode);

	DeferredLightingNode* LightNode = new DeferredLightingNode();
	RootNode->LinkToNode(LightNode);
	LightNode->GetInput(0)->SetLink(RootNode->GetOutput(0));
}
StorageNode* RenderGraph::AddStoreNode(StorageNode* Node)
{
	StoreNodes.push_back(Node);
	return Node;
}
