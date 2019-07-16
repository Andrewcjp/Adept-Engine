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
#include "Nodes/Flow/VRBranchNode.h"
#include "Core/Utils/StringUtil.h"
#include "Nodes/SSAONode.h"
#include "Nodes/UpdateReflectionsNode.h"
#include "Nodes/RayTraceReflectionsNode.h"
#include "UI/UIManager.h"
#include "Nodes/PathTraceSceneNode.h"

#define TESTVR 1
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

void RenderGraph::DebugOutput()
{
	UIManager::instance->RenderTextToScreen(3, "Graph: " + GraphName + " Info: N:" + std::to_string(ActiveNodeCount) + "/" + std::to_string(NodeCount) + " SN:" + std::to_string(StoreNodes.size()));
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
		if (IsVRGraph)
		{
			Node->FindVRContext();
		}
		NodeCount++;
		if (Node->IsNodeActive())
		{
			ActiveNodeCount++;
		}
		Node = Node->GetNextNode();
	}
	//PrintNodeData();
	ListNodes();
}
#define RUNRT 1
void RenderGraph::CreateDefTestgraph()
{
	GraphName = "Deferred Renderer";
	FrameBufferStorageNode* GBufferNode = AddStoreNode(new FrameBufferStorageNode());
	ShadowAtlasStorageNode* ShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateGBuffer(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	GBufferNode->SetFrameBufferDesc(Desc);

	SceneDataNode* SceneData = AddStoreNode(new SceneDataNode());
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	MainBuffer->SetFrameBufferDesc(Desc);

	FrameBufferStorageNode* SSAOBuffer = AddStoreNode(new FrameBufferStorageNode());
	Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	SSAOBuffer->SetFrameBufferDesc(Desc);

	GBufferNode->StoreType = EStorageType::Framebuffer;
	GBufferNode->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new GBufferWriteNode();
	RootNode->GetInput(0)->SetStore(GBufferNode);

	ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
	ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
	RootNode->LinkToNode(ShadowUpdate);

	UpdateReflectionsNode* UpdateProbesNode = new UpdateReflectionsNode();
	UpdateProbesNode->GetInput(0)->SetStore(ShadowDataNode);
	LinkNode(ShadowUpdate, UpdateProbesNode);
#if RUNRT
	FrameBufferStorageNode* RTXBuffer = AddStoreNode(new FrameBufferStorageNode());
	Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	RTXBuffer->SetFrameBufferDesc(Desc);

	RayTraceReflectionsNode* RTNode = new RayTraceReflectionsNode();
	LinkNode(UpdateProbesNode, RTNode);
	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetLink(RootNode->GetOutput(0));
	RTNode->GetInput(2)->SetStore(ShadowDataNode);
#endif
	DeferredLightingNode* LightNode = new DeferredLightingNode();

#if RUNRT
	LightNode->UseScreenSpaceReflection = true;
	LightNode->OnNodeSettingChange();
	LinkNode(RTNode, LightNode);
#else
	LinkNode(UpdateProbesNode, LightNode);
#endif
	LightNode->GetInput(0)->SetLink(RootNode->GetOutput(0));
	LightNode->GetInput(1)->SetStore(MainBuffer);
	LightNode->GetInput(2)->SetStore(SceneData);

	LightNode->GetInput(3)->SetStore(ShadowDataNode);

#if RUNRT
	LightNode->GetInput(4)->SetLink(RTNode->GetOutput(0));
#endif


	SSAONode* SSAO = new SSAONode();
	SSAO->GetInput(0)->SetLink(LightNode->GetOutput(0));
	SSAO->GetInput(1)->SetStore(GBufferNode);
	SSAO->GetInput(2)->SetStore(SSAOBuffer);
	LinkNode(LightNode, SSAO);

	PostProcessNode* PPNode = new PostProcessNode();
	LinkNode(SSAO, PPNode);
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

BranchNode * RenderGraph::AddBranchNode(RenderNode * Start, RenderNode * A, RenderNode * B, bool initalstate, std::string ExposeName/* = std::string()*/)
{
	BranchNode* Node = new BranchNode();
	Node->Conditonal = initalstate;
	Start->LinkToNode(Node);
	Node->LinkToNode(A);
	Node->BranchB = B;
	if (ExposeName.length() > 0)
	{
		ExposeItem(Node, ExposeName);
	}
	return Node;
}

void RenderGraph::LinkNode(RenderNode* A, RenderNode* B)
{
	A->LinkToNode(B);
}

void RenderGraph::CreateFWDGraph()
{
	GraphName = "Forward Renderer";
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
	ParticleSimulateNode* simNode = new ParticleSimulateNode();
	LinkNode(ShadowUpdate, simNode);
	ZPrePassNode* PreZ = new ZPrePassNode();
	ExposeItem(PreZ, "PREZ");

	LinkNode(simNode, PreZ);
	PreZ->GetInput(0)->SetStore(MainBuffer);
	ForwardRenderNode* FWDNode = new ForwardRenderNode();
	LinkNode(PreZ, FWDNode);
	FWDNode->UseLightCulling = false;
	FWDNode->UsePreZPass = false;
	FWDNode->UpdateSettings();
	FWDNode->GetInput(0)->SetStore(MainBuffer);
	FWDNode->GetInput(1)->SetStore(SceneData);
	FWDNode->GetInput(2)->SetStore(ShadowDataNode);
	ParticleRenderNode* renderNode = new ParticleRenderNode();
	LinkNode(FWDNode, renderNode);

	renderNode->GetInput(0)->SetStore(MainBuffer);
	OutputToScreenNode* Output = new OutputToScreenNode();

	DebugUINode* Debug = new DebugUINode();
	AddBranchNode(renderNode, Debug, Output, true, "Debug");
	Debug->GetInput(0)->SetLink(renderNode->GetOutput(0));
	LinkNode(Debug, Output);
	Output->GetInput(0)->SetLink(FWDNode->GetOutput(0));

}

void RenderGraph::ToggleCondition(const std::string & name)
{
	SetCondition(name, !GetCondition(name));
}

bool RenderGraph::SetCondition(std::string name, bool state)
{
	auto Itor = ExposedParms.find(name);
	if (Itor == ExposedParms.end())
	{
		Log::LogMessage("Failed to find prop \"" + name + "\"");
		return false;
	}
	Itor->second->SetState(state);
	return true;
}

bool RenderGraph::GetCondition(std::string name)
{
	auto Itor = ExposedParms.find(name);
	if (Itor == ExposedParms.end())
	{
		Log::LogMessage("Failed to find prop \"" + name + "\"");
		return false;
	}
	return Itor->second->GetState();
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

void RenderGraph::ListNodes()
{
	Log::LogMessage("---Debug Render Node List Start---");
	RenderNode* Node = RootNode;
	while (Node != nullptr)
	{
		if (Node->IsBranchNode())
		{
			Log::LogMessage(Node->GetName());
			Log::LogMessage("----- if true");
			RenderNode* APath = Node->GetNextNode();
			RenderNode* BPath = static_cast<BranchNode*>(Node)->BranchB;
			RenderNode* PathITor = BPath;
			while (PathITor != APath && PathITor != nullptr)
			{
				Log::LogMessage(PathITor->GetName());
				PathITor = PathITor->GetNextNode();
			}
			Log::LogMessage("----- if false");
		}
		else
		{
			if (Node->IsVRBranch())
			{
				if (static_cast<VRBranchNode*>(Node)->VrLoopBegin == nullptr)
				{
					Log::LogMessage("--- VR Branch loop start");
				}
				else
				{
					Log::LogMessage("--- VR Branch loop end");
				}
			}
			else
			{
				Log::LogMessage(Node->GetName());
			}
		}
		Node = Node->GetNextNode();
	}
	Log::LogMessage("---Exposed Settings---");
	for (auto it = ExposedParms.begin(); it != ExposedParms.end(); it++)
	{
		Log::LogMessage("Parm: " + it->second->name + " default value " + StringUtils::ToString(it->second->GetState()));
	}
	Log::LogMessage("---Debug Render Node List End---");

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

void RenderGraph::CreateVRFWDGraph()
{
	GraphName = "Forward VR Renderer";
	IsVRGraph = true;
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
	ParticleSimulateNode* simNode = new ParticleSimulateNode();
	LinkNode(ShadowUpdate, simNode);
	ZPrePassNode* PreZ = new ZPrePassNode();
	ExposeItem(PreZ, "Enable PreZ");


	VRBranchNode* VrStart = new VRBranchNode();

	LinkNode(simNode, VrStart);
	LinkNode(VrStart, PreZ);


	PreZ->GetInput(0)->SetStore(MainBuffer);
	ForwardRenderNode* FWDNode = new ForwardRenderNode();
	LinkNode(PreZ, FWDNode);
	FWDNode->UseLightCulling = false;
	FWDNode->UsePreZPass = false;
	FWDNode->UpdateSettings();
	FWDNode->GetInput(0)->SetStore(MainBuffer);
	FWDNode->GetInput(1)->SetStore(SceneData);
	FWDNode->GetInput(2)->SetStore(ShadowDataNode);

	ParticleRenderNode* renderNode = new ParticleRenderNode();
	LinkNode(FWDNode, renderNode);



	renderNode->GetInput(0)->SetStore(MainBuffer);
#if 0
	DebugUINode* Debug = new DebugUINode();
	AddBranchNode(FWDNode, simNode, Debug, true);
	renderNode->LinkToNode(Debug);
	Debug->GetInput(0)->SetLink(renderNode->GetOutput(0));
#endif
	OutputToScreenNode* Output = new OutputToScreenNode();

	VRBranchNode* VrEnd = new VRBranchNode();
	VrEnd->VrLoopBegin = VrStart;
	LinkNode(renderNode, VrEnd);
	LinkNode(VrEnd, Output);
	Output->GetInput(0)->SetLink(FWDNode->GetOutput(0));
}

void RenderGraph::ExposeItem(RenderNode* N, std::string name, bool Defaultstate /*= true*/)
{
	RenderGraphExposedSettings* Set = new RenderGraphExposedSettings(N, Defaultstate);
	Set->name = name;
	ExposedParms.emplace(name, Set);
}

void RenderGraphExposedSettings::SetState(bool state)
{
	if (Branch != nullptr)
	{
		Branch->Conditonal = state;
	}
	if (ToggleNode != nullptr)
	{
		ToggleNode->SetNodeActive(state);
	}
}

bool RenderGraphExposedSettings::GetState() const
{
	if (Branch != nullptr)
	{
		return Branch->Conditonal;
	}
	if (ToggleNode != nullptr)
	{
		return ToggleNode->IsNodeActive();
	}
	return false;
}


RenderGraphExposedSettings::RenderGraphExposedSettings(RenderNode * Node, bool Default)
{
	if (Node->IsBranchNode())
	{
		Branch = static_cast<BranchNode*>(Node);
	}
	else
	{
		ToggleNode = Node;
	}
	SetState(Default);
}


void RenderGraph::CreatePathTracedGraph()
{
	GraphName = "Path Traced Render";

	//SceneDataNode* SceneData = AddStoreNode(new SceneDataNode());
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	MainBuffer->SetFrameBufferDesc(Desc);

	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new PathTraceSceneNode();
	RootNode->GetInput(0)->SetStore(MainBuffer);
	
	OutputToScreenNode* Output = new OutputToScreenNode();
	LinkNode(RootNode, Output);
	Output->GetInput(0)->SetLink(RootNode->GetOutput(0));
}