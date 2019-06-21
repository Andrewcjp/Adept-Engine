#include "RenderGraph.h"
#include "NodeLink.h"
#include "Nodes/DeferredLightingNode.h"
#include "Nodes/GBufferWriteNode.h"
#include "StorageNodeFormats.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "Nodes/ForwardRenderNode.h"
#include "Nodes/OutputToScreenNode.h"
#include "StoreNodes/SceneDataNode.h"

RenderGraph::RenderGraph()
{}

RenderGraph::~RenderGraph()
{}

void RenderGraph::RunGraph()
{
	//Update causes all CPU side systems etc to be ready to render this frame
	Update();
	//Run the renderer!
	RootNode->ExecuteNode();
}

void RenderGraph::Resize()
{
	for (StorageNode* N : StoreNodes)
	{
		N->Resize();
	}
}

void RenderGraph::Update()
{
	for (StorageNode* N : StoreNodes)
	{
		N->Update();
	}
}

void RenderGraph::BuildGraph()
{
	ValidateGraph();
	for (StorageNode* N : StoreNodes)
	{
		N->CreateNode();
	}
	RenderNode* Node = RootNode;
	while (Node != nullptr)
	{
		Node->SetupNode();
		Node = Node->GetNextNode();
	}
	//PrintNodeData();
}

void RenderGraph::CreateDefTestgraph()
{
	FrameBufferStorageNode* GBufferNode = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateGBuffer(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	GBufferNode->SetFrameBufferDesc(Desc);

	GBufferNode->StoreType = EStorageType::Framebuffer;
	GBufferNode->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new GBufferWriteNode();
	RootNode->GetInput(0)->SetStore(GBufferNode);

	DeferredLightingNode* LightNode = new DeferredLightingNode();
	RootNode->LinkToNode(LightNode);
	LightNode->GetInput(0)->SetLink(RootNode->GetOutput(0));
}

void RenderGraph::CreateFWDGraph()
{
	SceneDataNode* SceneData = AddStoreNode(new SceneDataNode());


	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateGBuffer(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	MainBuffer->SetFrameBufferDesc(Desc);

	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	ForwardRenderNode* Node = new ForwardRenderNode();
	RootNode = Node;
	Node->UseLightCulling = false;
	Node->UsePreZPass = false;
	Node->UpdateSettings();
	RootNode->GetInput(0)->SetStore(MainBuffer);
	Node->GetInput(1)->SetStore(SceneData);

	OutputToScreenNode* Output = new OutputToScreenNode();
	RootNode->LinkToNode(Output);
	Output->GetInput(0)->SetLink(RootNode->GetOutput(0));

}

void RenderGraph::PrintNodeData()
{
	std::vector<std::string> Lines;
	std::string TileLine = "";
	RenderNode* Node = RootNode;
	const int NodeDistance = 10;
	std::string SpaceS = std::string(NodeDistance, ' ');
	std::string DashS = std::string(NodeDistance, '-');
	while (Node != nullptr)
	{
		TileLine += DashS + Node->GetName();
		for (uint i = 0; i < Node->GetNumInputs(); i++)
		{
			std::string Data = "I:" + Node->GetInput(i)->GetLinkName();
			if (Lines.size() <= i)
			{
				Lines.push_back(SpaceS + Data);
			}
			else
			{
				Lines[i] += SpaceS + Data;
			}
		}
		Node = Node->GetNextNode();
	}
	Log::LogMessage("---Debug Render Node Layout---");
	Log::LogMessage(TileLine);
	for (int i = 0; i < Lines.size(); i++)
	{
		Log::LogMessage(Lines[i]);
	}
}

void RenderGraph::ValidateGraph()
{
	ValidateArgs Validation = ValidateArgs();
	Validation.ErrorWrongFormat = false;
	RenderNode* Node = RootNode;
	while (Node != nullptr)
	{
		Node->ValidateNode(Validation);
		Node = Node->GetNextNode();
	}
	Log::LogMessage("Graph Validation Complete");
	for (std::string s : Validation.Errors)
	{
		Log::LogMessage(s, Log::Error);
	}
	for (std::string s : Validation.Warnings)
	{
		Log::LogMessage(s, Log::Warning);
	}
	ensure(!Validation.HasError());
}

void RenderGraph::ValidateArgs::AddWarning(std::string Message)
{
	Warnings.push_back(Message);
}

void RenderGraph::ValidateArgs::AddError(std::string Message)
{
	Errors.push_back(Message);
}

bool RenderGraph::ValidateArgs::HasError() const
{
	return Errors.size();
}
