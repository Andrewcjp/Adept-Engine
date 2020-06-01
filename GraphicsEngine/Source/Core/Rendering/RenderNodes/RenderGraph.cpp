#include "RenderGraph.h"
#include "Core/Utils/StringUtil.h"
#include "NodeLink.h"
#include "Nodes/OutputToScreenNode.h"
#include "Nodes/PathTraceSceneNode.h"
#include "Nodes/UpdateAccelerationStructuresNode.h"
#include "StorageNodeFormats.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "UI/UIManager.h"
#include "Nodes/UINode.h"
#include "Nodes/SimpleNode.h"
#include "Core/Performance/PerfManager.h"

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
	RenderNode* Node = RootNode;
	while (Node != nullptr)
	{
		Node->OnResourceResize();
		Node = Node->GetNextNode();
	}
	Log::LogMessage("RenderGraph Memory usage: " + StringUtils::ByteToMB(TotalResourceSize));
}

void RenderGraph::ResetForFrame()
{
	for (StorageNode* N : StoreNodes)
	{
		N->Reset();
	}
}

void RenderGraph::RefreshNodes()
{
	RenderNode* Node = RootNode;
	while (Node != nullptr)
	{
		Node->RefreshNode();
		Node = Node->GetNextNode();
	}
	if (GraphNeedsProcess)
	{
		Processor.Reset();
		Processor.Process(this);
		GraphNeedsProcess = false;
	}
}

void RenderGraph::Update()
{
	for (StorageNode* N : StoreNodes)
	{
		N->Update();
	}
	Drawer.Update();
	if (RHI::GetFrameCount() == 0)
	{
		Drawer.WriteGraphViz(this);
	}
	Drawer.Draw(this);
	RefreshNodes();
}

void RenderGraph::BuildGraph()
{
#if EDITORUI
	ApplyEditorToGraph();
#endif
	Log::LogMessage("Building graph \"" + GraphName + "\"");
	ValidateGraph();
	ensureMsgf(RootNode, "No root node is set");
	RenderNode* Node = RootNode;
	{
		SCOPE_STARTUP_COUNTER("RG CreateGraph");
		while (Node != nullptr)
		{
			Node->OnGraphCreate();
			Node = Node->GetNextNode();
		}
	}
	PerfManager::Get()->FlushSingleActionTimer("RG CreateGraph", true);
	Processor.Process(this);
	
	GraphNeedsProcess = false;
	for (StorageNode* N : StoreNodes)
	{
		N->CreateNode();
	}
	Node = RootNode;
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

	ListNodes();
}


FrameBufferStorageNode* RenderGraph::CreateRTXBuffer()
{
	FrameBufferStorageNode* RTXBuffer = AddStoreNode(new FrameBufferStorageNode("RTX Buffer"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.LinkToBackBufferScaleFactor = 1.0f;
	Desc.AllowUnorderedAccess = true;
	Desc.SimpleStartingState = EResourceState::Non_PixelShader;
	Desc.clearcolour = glm::vec4(0, 0, 0, 0);
	RTXBuffer->SetFrameBufferDesc(Desc);
	return RTXBuffer;
}

void RenderGraph::EndGraph(FrameBufferStorageNode* MainBuffer, RenderNode* Output)
{
#if 0
	NodeLink* BufferLink = nullptr;
	SimpleNode* UIN = new SimpleNode("UI",
		[&](SimpleNode* N)
	{
		BufferLink = N->AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DontCare, "Main RT");
	},
		[](RHICommandList* list)
	{
		UIManager::Get()->RenderWidgets(list);
	});
	BufferLink->SetStore(MainBuffer);
	LinkNode(Output, UIN);
#else
	UINode* UIRender = new UINode();
	UIRender->GetInput(0)->SetStore(MainBuffer);
	LinkNode(Output, UIRender);
#endif
}

void RenderGraph::LinkNode(RenderNode* A, RenderNode* B)
{
	A->LinkToNode(B);
}

void RenderGraph::InsertNode(RenderNode* Point, RenderNode* B)
{
	RenderNode* N = Point->GetNextNode();
	Point->LinkToNode(B);
	B->LinkToNode(N);
}

void RenderGraph::ToggleCondition(const std::string& name)
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
	Itor->second->CVar->SetValue(state);
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
			//			RenderNode* APath = Node->GetNextNode();
			//			RenderNode* BPath = static_cast<BranchNode*>(Node)->BranchB;
				//		RenderNode* PathITor = BPath;
						//while (PathITor != APath && PathITor != nullptr)
						//{
						//	Log::LogMessage(PathITor->GetName());
						//	PathITor = PathITor->GetNextNode();
						//}
			Log::LogMessage("----- if false");
		}
		else
		{
			if (Node->IsVRBranch())
			{
				/*if (static_cast<VRBranchNode*>(Node)->VrLoopBegin == nullptr)
				{
					Log::LogMessage("--- VR Branch loop start");
				}
				else
				{
					Log::LogMessage("--- VR Branch loop end");
				}*/
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
	Validation.TargetGraph = this;
	Validation.ErrorWrongFormat = false;
	RenderNode* Node = RootNode;
	while (Node != nullptr)
	{
		Node->ValidateNode(Validation);
		Node = Node->GetNextNode();
	}
	Log::LogMessage("Graph Validation Complete");
	for (std::pair<std::string, Log::Severity> p : Validation.OutputLog)
	{
		Log::LogMessage(p.first, p.second);
	}
	ensure(!Validation.HasError());
}

void RenderGraph::ValidateArgs::AddWarning(std::string Message)
{
	OutputLog.push_back(std::make_pair(Message, Log::Warning));
}

void RenderGraph::ValidateArgs::AddError(std::string Message)
{
	OutputLog.push_back(std::make_pair(Message, Log::Error));
	Error = true;
}

bool RenderGraph::ValidateArgs::HasError() const
{
	return Error;
}


bool RenderGraph::IsGraphValid()
{
	if (RootNode == nullptr)
	{
		return false;
	}
	if (IsVRGraph && RHI::GetRenderSettings()->VRHMDMode == EVRHMDMode::Disabled)
	{
		return false;
	}
	if (RequiresMGPU && RHI::GetDeviceCount() == 1)
	{
		return false;
	}
	if (RequiresRT && !RHI::GetRenderSettings()->RaytracingEnabled())
	{
		return false;
	}
	return true;
}

RenderNode* RenderGraph::FindFirstOf(const std::string& name)
{
	RenderNode* itor = RootNode;
	while (itor != nullptr)
	{
		if (itor->GetName() == name)
		{
			return itor;
		}
		itor = itor->GetNextNode();
	}
	return nullptr;
}

std::vector<RenderNode*> RenderGraph::FindAllOf(const std::string& name)
{
	std::vector<RenderNode*> out;
	RenderNode* itor = RootNode;
	while (itor->GetNextNode() != nullptr)
	{
		if (itor->GetName() == name)
		{
			out.push_back(itor);
		}
		itor = itor->GetNextNode();
	}
	return out;
}

RenderNode* RenderGraph::GetNodeAtIndex(int i)
{
	RenderNode* itor = RootNode;
	int index = 0;
	while (itor->GetNextNode() != nullptr)
	{
		if (index >= i)
		{
			return itor;
		}
		itor = itor->GetNextNode();
		index++;
	}
	return nullptr;
}

int RenderGraph::GetIndexOfNode(RenderNode* Node)
{
	RenderNode* itor = RootNode;
	int index = 0;
	while (itor != nullptr)
	{
		if (itor == Node)
		{
			return index;
		}
		itor = itor->GetNextNode();
		index++;
	}
	return -1;
}

void RenderGraph::RunTests()
{
#if 0 //RUNTESTS
	if (!RHI::GetRenderSettings()->SelectedGraph == EBuiltinRenderGraphs::DeferredRenderer)
	{
		return;
	}
	RenderNode* Lnode = FindFirstOf(DeferredLightingNode::GetNodeName());
	ensure(Lnode);
	std::vector<RenderNode*> Lnodes = FindAllOf(DeferredLightingNode::GetNodeName());
	ensure(Lnodes.size() > 0);
	Lnode = GetNodeAtIndex(3);
	ensure(Lnode);
#endif
}

void RenderGraph::InvalidateGraph()
{
	GraphNeedsProcess = true;
}

RenderNode* RenderGraph::AppendNode(RenderNode* node)
{
	if (HeadNode == nullptr)
	{
		HeadNode = RootNode;
	}
	HeadNode->LinkToNode(node);
	HeadNode = node;
	return HeadNode;
}

void RenderGraph::ExposeItem(RenderNode* N, std::string name, bool Defaultstate /*= true*/)
{
	RenderGraphExposedSettings* Set = new RenderGraphExposedSettings(N, Defaultstate);
	Set->CVar = new ConsoleVariable("rg." + name, 0);
	Set->CVar->OnChangedBoolFunction = std::bind(&RenderGraphExposedSettings::SetState, Set, std::placeholders::_1);
	Set->name = name;
	Set->CVar->SetValue(Defaultstate);
	Set->SetState(Defaultstate);
	ExposedParms.emplace(name, Set);
}

void RenderGraph::ExposeNodeOption(RenderNode* N, std::string name, bool* data, bool Defaultstate)
{
	RenderGraphExposedSettings* Set = new RenderGraphExposedSettings(N, Defaultstate);
	Set->CVar = new ConsoleVariable("rg." + name, 0);
	Set->CVar->OnChangedBoolFunction = std::bind(&RenderGraphExposedSettings::SetState, Set, std::placeholders::_1);
	Set->name = name;
	Set->CVar->SetValue(Defaultstate);
	Set->SetState(Defaultstate);
	Set->TargetProp = data;
	ExposedParms.emplace(name, Set);
}

void RenderGraphExposedSettings::SetState(bool state)
{
	if (Branch != nullptr)
	{
		//		Branch->Conditonal = state;
	}
	if (TargetProp != nullptr)
	{
		*TargetProp = state;
	}
	else
	{
		if (ToggleNode != nullptr)
		{
			ToggleNode->SetNodeActive(state);
		}
	}
}

bool RenderGraphExposedSettings::GetState() const
{
	if (Branch != nullptr)
	{
		//		return Branch->Conditonal;
	}
	if (ToggleNode != nullptr)
	{
		return ToggleNode->IsNodeActive();
	}
	return false;
}


RenderGraphExposedSettings::RenderGraphExposedSettings(RenderNode* Node, bool Default)
{
	if (Node->IsBranchNode())
	{
		//Branch = static_cast<BranchNode*>(Node);
	}
	else
	{
		ToggleNode = Node;
	}
}


void RenderGraph::CreatePathTracedGraph()
{
	GraphName = "Path Traced Render";
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	MainBuffer->SetFrameBufferDesc(Desc);

	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new UpdateAccelerationStructuresNode();

	PathTraceSceneNode* PathTraceNode = new PathTraceSceneNode();
	LinkNode(RootNode, PathTraceNode);
	PathTraceNode->GetInput(0)->SetStore(MainBuffer);

	OutputToScreenNode* Output = new OutputToScreenNode();
	Output->GetInput(0)->SetStore(MainBuffer);
	LinkNode(PathTraceNode, Output);	
}

void RenderGraph::CreateRaytracingGraph()
{
	GraphName = "Ray Traced view";
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	MainBuffer->SetFrameBufferDesc(Desc);

	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new UpdateAccelerationStructuresNode();

	PathTraceSceneNode* PathTraceNode = new PathTraceSceneNode();
	LinkNode(RootNode, PathTraceNode);
	PathTraceNode->GetInput(0)->SetStore(MainBuffer);

	OutputToScreenNode* Output = new OutputToScreenNode();
	Output->GetInput(0)->SetStore(MainBuffer);
	LinkNode(PathTraceNode, Output);
}

void RenderGraph::CreateVoxelTracingGraph()
{
	GraphName = "Voxel Traced view";
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	MainBuffer->SetFrameBufferDesc(Desc);

	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	RootNode = new UpdateAccelerationStructuresNode();

	PathTraceSceneNode* PathTraceNode = new PathTraceSceneNode();
	LinkNode(RootNode, PathTraceNode);
	PathTraceNode->GetInput(0)->SetStore(MainBuffer);

	OutputToScreenNode* Output = new OutputToScreenNode();
	Output->GetInput(0)->SetStore(MainBuffer);
	LinkNode(PathTraceNode, Output);
}

std::vector<StorageNode*> RenderGraph::GetNodesOfType(EStorageType::Type type)
{
	std::vector<StorageNode*> Out;
	for (int i = 0; i < StoreNodes.size(); i++)
	{
		if (StoreNodes[i]->StoreType == type)
		{
			Out.push_back(StoreNodes[i]);
		}
	}
	return Out;
}

void RG_PatchMarkerCollection::AddPatchSet(RG_PatchSet* patch)
{
	Sets.push_back(patch);
}

void RG_PatchMarkerCollection::AddSinglePatch(PatchBase* patch, EBuiltInRenderGraphPatch::Type type)
{
	RG_PatchSet* Set = new RG_PatchSet();
	Set->AddPatchMarker(patch, type);
	AddPatchSet(Set);
}

RG_PatchSet* RenderGraph::FindMarker(EBuiltInRenderGraphPatch::Type type)
{
	for (int i = 0; i < Markers.Sets.size(); i++)
	{
		if (Markers.Sets[i]->SupportsPatchType(type))
		{
			return Markers.Sets[i];
		}
	}
	return nullptr;
}


bool RG_PatchSet::SupportsPatchType(EBuiltInRenderGraphPatch::Type type)
{
	return VectorUtils::Contains(SupportedPatches, type);
}

void RG_PatchSet::AddPatchMarker(PatchBase* patch, EBuiltInRenderGraphPatch::Type type)
{
	Markers.push_back(patch);
	SupportedPatches.push_back(type);
}
