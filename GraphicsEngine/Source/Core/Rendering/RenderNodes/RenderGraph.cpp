#include "RenderGraph.h"
#include "NodeLink.h"
#include "Nodes/DeferredLightingNode.h"
#include "Nodes/GBufferWriteNode.h"
#include "StorageNodeFormats.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "Nodes/ForwardRenderNode.h"
#include "Nodes/OutputToScreenNode.h"
#include "StoreNodes/SceneDataNode.h"
#include "Nodes/ParticleSimulateNode.h"
#include "Nodes/ParticleRenderNode.h"
#include "Nodes/DebugUINode.h"
#include "Nodes/PostProcessNode.h"
#include "Nodes/ShadowUpdateNode.h"
#include "StoreNodes/ShadowAtlasStorageNode.h"
#include "Nodes/ZPrePassNode.h"
#include "Nodes/Flow/BranchNode.h"
#include "Nodes/VisModeNode.h"

RenderGraph::RenderGraph()
{}

RenderGraph::~RenderGraph()
{
	DestoryGraph();
}

void RenderGraph::DestoryGraph()
{
	MemoryUtils::DeleteVector(StoreNodes);
	std::vector<RenderNode*> Nodes;
	RenderNode* Node = RootNode;
	while (Node != nullptr)
	{
		Nodes.push_back(Node);
		Node = Node->GetNextNode();
	}
	MemoryUtils::DeleteVector(Nodes);
	RootNode = nullptr;
}

void RenderGraph::RunGraph()
{
	//reset nodes
	ResetForFrame();
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

void RenderGraph::ResetForFrame()
{
	for (StorageNode* N : StoreNodes)
	{
		N->Reset();
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
	ShadowAtlasStorageNode* ShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateGBuffer(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	GBufferNode->SetFrameBufferDesc(Desc);

	SceneDataNode* SceneData = AddStoreNode(new SceneDataNode());
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	MainBuffer->SetFrameBufferDesc(Desc);


	GBufferNode->StoreType = EStorageType::Framebuffer;
	GBufferNode->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new GBufferWriteNode();
	RootNode->GetInput(0)->SetStore(GBufferNode);

	ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
	ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
	RootNode->LinkToNode(ShadowUpdate);


	DeferredLightingNode* LightNode = new DeferredLightingNode();
	ShadowUpdate->LinkToNode(LightNode);
	LightNode->GetInput(0)->SetLink(RootNode->GetOutput(0));
	LightNode->GetInput(1)->SetStore(MainBuffer);
	LightNode->GetInput(2)->SetStore(SceneData);

	LightNode->GetInput(3)->SetStore(ShadowDataNode);

	PostProcessNode* PPNode = new PostProcessNode();
	LightNode->LinkToNode(PPNode);
	PPNode->GetInput(0)->SetLink(LightNode->GetOutput(0));

	DebugUINode* Debug = new DebugUINode();
	PPNode->LinkToNode(Debug);
	Debug->GetInput(0)->SetLink(PPNode->GetOutput(0));

	VisModeNode* VisNode = new VisModeNode();
	VisNode->GetInput(0)->SetLink(Debug->GetOutput(0));
	VisNode->GetInput(1)->SetStore(GBufferNode);
	Debug->LinkToNode(VisNode);

	OutputToScreenNode* Output = new OutputToScreenNode();
	VisNode->LinkToNode(Output);
	Output->GetInput(0)->SetLink(VisNode->GetOutput(0));
}

BranchNode * RenderGraph::AddBranchNode(RenderNode * Start, RenderNode * A, RenderNode * B, bool initalstate)
{
	BranchNode* Node = new BranchNode();
	Node->Conditonal = initalstate;
	Start->LinkToNode(Node);
	Node->LinkToNode(A);
	Node->BranchB = B;
	BranchNodes.push_back(Node);
	return Node;
}

void RenderGraph::CreateFWDGraph()
{
	SceneDataNode* SceneData = AddStoreNode(new SceneDataNode());
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	MainBuffer->SetFrameBufferDesc(Desc);

	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;

	ShadowAtlasStorageNode* ShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
	ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
	RootNode = ShadowUpdate;
	ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);

	ZPrePassNode* PreZ = new ZPrePassNode();
	//PreZ->SetNodeActive(true);
	OptionNode = PreZ;
	RootNode->LinkToNode(PreZ);
	PreZ->GetInput(0)->SetStore(MainBuffer);
	ForwardRenderNode* FWDNode = new ForwardRenderNode();
	PreZ->LinkToNode(FWDNode);
	FWDNode->UseLightCulling = false;
	FWDNode->UsePreZPass = false;
	FWDNode->UpdateSettings();
	FWDNode->GetInput(0)->SetStore(MainBuffer);
	FWDNode->GetInput(1)->SetStore(SceneData);
	FWDNode->GetInput(2)->SetStore(ShadowDataNode);



	ParticleSimulateNode* simNode = new ParticleSimulateNode();
	FWDNode->LinkToNode(simNode);

	ParticleRenderNode* renderNode = new ParticleRenderNode();
	simNode->LinkToNode(renderNode);



	renderNode->GetInput(0)->SetStore(MainBuffer);

	DebugUINode* Debug = new DebugUINode();
	AddBranchNode(FWDNode, simNode, Debug, true);
	renderNode->LinkToNode(Debug);
	Debug->GetInput(0)->SetLink(renderNode->GetOutput(0));

	OutputToScreenNode* Output = new OutputToScreenNode();
	Debug->LinkToNode(Output);
	Output->GetInput(0)->SetLink(FWDNode->GetOutput(0));

}

void RenderGraph::SetCondition(int nodeIndex, bool state)
{
	BranchNodes[nodeIndex]->Conditonal = state;
}

bool RenderGraph::GetCondition(int nodeIndex)
{
	return BranchNodes[nodeIndex]->Conditonal;
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
