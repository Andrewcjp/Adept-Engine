#include "RenderGraph.h"
#include "Core/Utils/StringUtil.h"
#include "NodeLink.h"
#include "Nodes/DebugUINode.h"
#include "Nodes/DeferredLightingNode.h"
#include "Nodes/Flow/BranchNode.h"
#include "Nodes/Flow/VRBranchNode.h"
#include "Nodes/ForwardRenderNode.h"
#include "Nodes/GBufferWriteNode.h"
#include "Nodes/OutputToScreenNode.h"
#include "Nodes/ParticleRenderNode.h"
#include "Nodes/ParticleSimulateNode.h"
#include "Nodes/PathTraceSceneNode.h"
#include "Nodes/PostProcessNode.h"
#include "Nodes/RayTraceReflectionsNode.h"
#include "Nodes/ShadowUpdateNode.h"
#include "Nodes/SSAONode.h"
#include "Nodes/UpdateAccelerationStructuresNode.h"
#include "Nodes/UpdateReflectionsNode.h"
#include "Nodes/VisModeNode.h"
#include "Nodes/ZPrePassNode.h"
#include "StorageNodeFormats.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "StoreNodes/SceneDataNode.h"
#include "StoreNodes/ShadowAtlasStorageNode.h"
#include "UI/UIManager.h"
#include "StoreNodes/InterGPUStorageNode.h"
#include "Nodes/InterGPUCopyNode.h"
#include "Nodes/SubmitToHMDNode.h"
#include "RenderGraphProcessor.h"
#include "Nodes/LightCullingNode.h"
#include "Nodes/VRXShadingRateNode.h"
#include "Nodes/ShadowMaskNode.h"
#include "Nodes/VoxelReflectionsNode.h"

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
}

void RenderGraph::BuildGraph()
{
#if WITH_EDITOR
	ApplyEditorToGraph();
#endif
	Log::LogMessage("Building graph \"" + GraphName + "\"");
	ValidateGraph();
	ensureMsgf(RootNode, "No root node is set");
	Processor.Process(this);
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
	ListNodes();
}

void RenderGraph::ApplyEditorToGraph()
{
	GraphName += "(Editor)";
	OutputToScreenNode* OutputNode = (OutputToScreenNode*)FindFirstOf(OutputToScreenNode::GetNodeName());
	FrameBufferStorageNode* FinalCompostBuffer = AddStoreNode(new FrameBufferStorageNode("Editor Output"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	FinalCompostBuffer->SetFrameBufferDesc(Desc);
	FinalCompostBuffer->SetRetained();
	OutputNode->GetInput(1)->SetStore(FinalCompostBuffer);
	//the editor composts to this temp buffer, which is rendered by the UI
}

void RenderGraph::AddVRXSupport()
{
	GraphName += "(VRX)";
	FrameBufferStorageNode* VRXShadingRateImage = AddStoreNode(new FrameBufferStorageNode("VRX Image"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R8_UINT;
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale_TileSize;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	Desc.SimpleStartingState = EResourceState::UAV;
	Desc.AllowUnorderedAccess = true;
	Desc.LinkToBackBufferScaleFactor = RHI::GetDefaultDevice()->GetCaps().VRSTileSize;
	VRXShadingRateImage->SetFrameBufferDesc(Desc);
	RenderNode* LightingNode = FindFirstOf(DeferredLightingNode::GetNodeName());
	VRXShadingRateNode* RateNode = new VRXShadingRateNode();
	RateNode->GetInput(0)->SetStore(VRXShadingRateImage);
	LightingNode->GetLastNode()->LinkToNode(RateNode);
	RateNode->LinkToNode(LightingNode);
	NodeLink* link = LightingNode->GetInputLinkByName("VRX Image");
	if (link != nullptr)
	{
		link->SetLink(RateNode->GetInput(0));	
	}
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR())
	{
		RenderNode* pp = FindFirstOf(PostProcessNode::GetNodeName());
		if (pp != nullptr)
		{
			pp->GetInput(1)->SetStore(VRXShadingRateImage);
		}
	}
		RenderNode* gbufferwrite = FindFirstOf(GBufferWriteNode::GetNodeName());
	if (gbufferwrite != nullptr)
	{
		RateNode->GetInput(1)->SetLink(gbufferwrite->GetOutput(0));
	}
	RenderNode* shadowmask = FindFirstOf(ShadowMaskNode::GetNodeName());
	if (shadowmask != nullptr)
	{
		RateNode->GetInput(2)->SetLink(shadowmask->GetOutput(0));
	}	
}

void RenderGraph::CreateDefGraphWithRT()
{
	RequiresRT = true;
	CreateDefTestgraph();
	GraphName += "(RT)";
	//find nodes
	DeferredLightingNode* LightNode = RenderNode::NodeCast<DeferredLightingNode>(FindFirstOf(DeferredLightingNode::GetNodeName()));
	GBufferWriteNode* gbuffer = RenderNode::NodeCast<GBufferWriteNode>(FindFirstOf(GBufferWriteNode::GetNodeName()));
	ShadowAtlasStorageNode* ShadowDataNode = StorageNode::NodeCast<ShadowAtlasStorageNode>(GetNodesOfType(EStorageType::ShadowData)[0]);
	RenderNode* UpdateProbesNode = FindFirstOf(ParticleSimulateNode::GetNodeName());

	//then insert
	FrameBufferStorageNode* RTXBuffer = AddStoreNode(new FrameBufferStorageNode("RTX Buffer"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	//Desc.LinkToBackBufferScaleFactor = 2.0f;
	Desc.AllowUnorderedAccess = true;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	RTXBuffer->SetFrameBufferDesc(Desc);
	UpdateAccelerationStructuresNode* UpdateAcceleration = new UpdateAccelerationStructuresNode();

	LinkNode(UpdateProbesNode, UpdateAcceleration);
	RayTraceReflectionsNode* RTNode = new RayTraceReflectionsNode();
	LinkNode(UpdateAcceleration, RTNode);
	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetLink(gbuffer->GetOutput(0));
	RTNode->GetInput(2)->SetStore(ShadowDataNode);
	ExposeItem(RTNode, StandardSettings::UseRaytrace);

	LightNode->UseScreenSpaceReflection = true;

	//LightNode->UpdateSettings();
	ExposeNodeOption(LightNode, StandardSettings::UseSSR, &LightNode->UseScreenSpaceReflection, true);
	LinkNode(RTNode, LightNode);
	LightNode->GetInput(4)->SetLink(RTNode->GetOutput(0));
}

void RenderGraph::CreateDefGraphWithVoxelRT()
{
	//RequiresRT = true;
	CreateDefTestgraph();
	GraphName += "(Voxel)";
	//find nodes
	DeferredLightingNode* LightNode = RenderNode::NodeCast<DeferredLightingNode>(FindFirstOf(DeferredLightingNode::GetNodeName()));
	GBufferWriteNode* gbuffer = RenderNode::NodeCast<GBufferWriteNode>(FindFirstOf(GBufferWriteNode::GetNodeName()));
	//ShadowAtlasStorageNode* ShadowDataNode = StorageNode::NodeCast<ShadowAtlasStorageNode>(GetNodesOfType(EStorageType::ShadowData)[0]);
	RenderNode* UpdateProbesNode = FindFirstOf(ParticleSimulateNode::GetNodeName());

	//then insert
	FrameBufferStorageNode* RTXBuffer = AddStoreNode(new FrameBufferStorageNode("RTX Buffer"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.LinkToBackBufferScaleFactor = 1.0f;
	Desc.AllowUnorderedAccess = true;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	RTXBuffer->SetFrameBufferDesc(Desc);
	//UpdateAccelerationStructuresNode* UpdateAcceleration = new UpdateAccelerationStructuresNode();

	VoxelReflectionsNode* RTNode = new VoxelReflectionsNode();
	LinkNode(UpdateProbesNode, RTNode);
	//LinkNode(UpdateAcceleration, RTNode);
	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetLink(gbuffer->GetOutput(0));
	//RTNode->GetInput(2)->SetStore(ShadowDataNode);
	ExposeItem(RTNode, StandardSettings::UseRaytrace);

	LightNode->UseScreenSpaceReflection = true;

	//LightNode->UpdateSettings();
	ExposeNodeOption(LightNode, StandardSettings::UseSSR, &LightNode->UseScreenSpaceReflection, true);
	LinkNode(RTNode, LightNode);
	LightNode->GetInput(4)->SetLink(RTNode->GetOutput(0));
	
}

void RenderGraph::CreateDefTestgraph()
{
	GraphName = "Deferred Renderer";
	FrameBufferStorageNode* GBufferNode = AddStoreNode(new FrameBufferStorageNode("GBuffer"));
	ShadowAtlasStorageNode* ShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateGBuffer(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	GBufferNode->SetFrameBufferDesc(Desc);

	SceneDataNode* SceneData = AddStoreNode(new SceneDataNode());
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode("Output Buffer"));
	Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	//if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR())
	{
		Desc.NeedsDepthStencil = true;
		Desc.DepthFormat = eTEXTURE_FORMAT::FORMAT_D24_UNORM_S8_UINT;
	}
#if 1
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R16G16B16A16_FLOAT;
#endif
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
#if TEST_VRS
	Desc.VarRateSettings.BufferMode = FrameBufferVariableRateSettings::VRR;
#endif
	Desc.AllowDynamicResize = true;
	MainBuffer->SetFrameBufferDesc(Desc);
	MainBuffer->SetRetained();

	FrameBufferStorageNode* SSAOBuffer = AddStoreNode(new FrameBufferStorageNode("SSAO Buffer"));
	Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.AllowUnorderedAccess = true;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	SSAOBuffer->SetFrameBufferDesc(Desc);

	FrameBufferStorageNode* ShadowMaskBuffer = AddStoreNode(new FrameBufferStorageNode("Shadow Mask"));
	Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.SimpleStartingState = EResourceState::RenderTarget;
	ShadowMaskBuffer->SetFrameBufferDesc(Desc);
	ShadowMaskNode* MaskNode = new ShadowMaskNode();


	ZPrePassNode* PreZ = new ZPrePassNode();
	RootNode = PreZ;
	ExposeItem(PreZ, StandardSettings::UsePreZ);
	PreZ->GetInput(0)->SetStore(GBufferNode);


	GBufferNode->StoreType = EStorageType::Framebuffer;
	GBufferNode->DataFormat = StorageFormats::DefaultFormat;
	GBufferWriteNode* WriteNode = new GBufferWriteNode();
	LinkNode(PreZ, WriteNode);
	WriteNode->GetInput(0)->SetLink(PreZ->GetOutput(0));

	ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
	ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
	WriteNode->LinkToNode(ShadowUpdate);

	MaskNode->GetInput(0)->SetStore(ShadowMaskBuffer);
	MaskNode->GetInput(1)->SetStore(ShadowDataNode);
	MaskNode->GetInput(2)->SetLink(WriteNode->GetOutput(0));
	LinkNode(ShadowUpdate, MaskNode);

	UpdateReflectionsNode* UpdateProbesNode = new UpdateReflectionsNode();
	UpdateProbesNode->GetInput(0)->SetStore(ShadowDataNode);
	LinkNode(MaskNode, UpdateProbesNode);

	DeferredLightingNode* LightNode = new DeferredLightingNode();
	ParticleSimulateNode* ParticleSimNode = new ParticleSimulateNode();

#if 0
	LightCullingNode* LightCull = new LightCullingNode();
	LinkNode(UpdateProbesNode, LightCull);
	LinkNode(LightCull, ParticleSimNode);
#else
	LinkNode(UpdateProbesNode, ParticleSimNode);
#endif	
	LinkNode(ParticleSimNode, LightNode);

	LightNode->GetInput(0)->SetLink(WriteNode->GetOutput(0));
	LightNode->GetInput(1)->SetStore(MainBuffer);
	LightNode->GetInput(2)->SetStore(SceneData);
	LightNode->GetInput(3)->SetStore(ShadowDataNode);
	LightNode->GetInputLinkByName("ShadowMask")->SetLink(MaskNode->GetOutput(0));

	ParticleRenderNode* PRenderNode = new ParticleRenderNode();
	LinkNode(LightNode, PRenderNode);
	PRenderNode->GetInput(0)->SetLink(LightNode->GetOutput(0));
	PRenderNode->GetInput(1)->SetStore(GBufferNode);

	SSAONode* SSAO = new SSAONode();
	SSAO->GetInput(0)->SetLink(LightNode->GetOutput(0));
	SSAO->GetInput(1)->SetLink(WriteNode->GetOutput(0));
	SSAO->GetInput(2)->SetStore(SSAOBuffer);
	LinkNode(PRenderNode, SSAO);

	PostProcessNode* PPNode = new PostProcessNode();
	LinkNode(SSAO, PPNode);
	PPNode->GetInput(0)->SetLink(PRenderNode->GetOutput(0));

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

	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRX())
	{
		AddVRXSupport();
	}
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
	Desc.VarRateSettings.BufferMode = FrameBufferVariableRateSettings::VRR;
	Desc.AllowUnorderedAccess = true;
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
	ExposeItem(PreZ, StandardSettings::UsePreZ);

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
	PostProcessNode* PP = new PostProcessNode();
	PP->GetInput(0)->SetStore(MainBuffer);
	LinkNode(renderNode, PP);

	OutputToScreenNode* Output = new OutputToScreenNode();

	DebugUINode* Debug = new DebugUINode();
	AddBranchNode(PP, Debug, Output, true, "Debug");
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
	Validation.TargetGraph = this;
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
	Desc.AllowUnorderedAccess = true;
	Desc.VarRateSettings.BufferMode = FrameBufferVariableRateSettings::VRR;
	MainBuffer->SetFrameBufferDesc(Desc);
	MainBuffer->Name = "Main Buffer";
	MainBuffer->IsVRFramebuffer = true;
	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;

	ShadowAtlasStorageNode* ShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
	ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
	RootNode = ShadowUpdate;
	ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
	ParticleSimulateNode* simNode = new ParticleSimulateNode();
	LinkNode(ShadowUpdate, simNode);
	ZPrePassNode* PreZ = new ZPrePassNode();
	ExposeItem(PreZ, StandardSettings::UsePreZ);

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

	PostProcessNode* PP = new PostProcessNode();
	PP->GetInput(0)->SetStore(MainBuffer);
	LinkNode(renderNode, PP);

	DebugUINode* Debug = new DebugUINode();
	LinkNode(PP, Debug);
	Debug->GetInput(0)->SetLink(renderNode->GetOutput(0));

	OutputToScreenNode* Output = new OutputToScreenNode();
	SubmitToHMDNode* SubNode = new SubmitToHMDNode();
	SubNode->GetInput(0)->SetLink(FWDNode->GetOutput(0));
	LinkNode(Debug, SubNode);

	VRBranchNode* VrEnd = new VRBranchNode();
	VrEnd->VrLoopBegin = VrStart;
	LinkNode(SubNode, VrEnd);
	LinkNode(VrEnd, Output);
	Output->GetInput(0)->SetLink(FWDNode->GetOutput(0));
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

RenderNode * RenderGraph::GetNodeAtIndex(int i)
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

void RenderGraph::ExposeNodeOption(RenderNode * N, std::string name, bool * data, bool Defaultstate)
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
		Branch->Conditonal = state;
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
	LinkNode(PathTraceNode, Output);
	Output->GetInput(0)->SetLink(PathTraceNode->GetOutput(0));
}

void RenderGraph::CreateFallbackGraph()
{
	GraphName = "Fallback Renderer";
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	MainBuffer->SetFrameBufferDesc(Desc);
	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	OutputToScreenNode* Output = new OutputToScreenNode();
	DebugUINode* Debug = new DebugUINode();
	Debug->ClearBuffer = true;
	RootNode = Debug;
	Debug->GetInput(0)->SetStore(MainBuffer);
	LinkNode(Debug, Output);
	Output->GetInput(0)->SetLink(Debug->GetOutput(0));
}
void RenderGraph::CreateMGPU_TESTGRAPH()
{
	RequiresMGPU = true;
	GraphName = "Mgpu tester";
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	MainBuffer->SetFrameBufferDesc(Desc);
	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	OutputToScreenNode* Output = new OutputToScreenNode();
	DebugUINode* Debug = new DebugUINode();
	Debug->SetDevice(RHI::GetDeviceContext(0));
	RootNode = Debug;
	Debug->GetInput(0)->SetStore(MainBuffer);
	Debug->ClearBuffer = true;
	Output->GetInput(0)->SetLink(Debug->GetOutput(0));

	FrameBufferStorageNode* OtherGPUCopy = AddStoreNode(new FrameBufferStorageNode());
	OtherGPUCopy->SetDevice(RHI::GetDeviceContext(1));
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_COPY_DEST;
	OtherGPUCopy->SetFrameBufferDesc(Desc);

	FrameBufferStorageNode* MainGPU = AddStoreNode(new FrameBufferStorageNode());
	MainGPU->SetDevice(RHI::GetDeviceContext(0));
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_COPY_DEST;
	MainGPU->SetFrameBufferDesc(Desc);

	InterGPUStorageNode* HostStore = AddStoreNode(new InterGPUStorageNode());
	HostStore->StoreTargets.push_back(OtherGPUCopy);

	InterGPUCopyNode* CopyTo = new InterGPUCopyNode(RHI::GetDeviceContext(1));
	CopyTo->CopyTo = true;
	CopyTo->GetInput(0)->SetStore(OtherGPUCopy);
	CopyTo->GetInput(1)->SetStore(HostStore);
	InterGPUCopyNode* CopyFrom = new InterGPUCopyNode(RHI::GetDeviceContext(0));
	CopyFrom->CopyTo = false;

	CopyFrom->GetInput(0)->SetStore(MainGPU);
	CopyFrom->GetInput(1)->SetStore(HostStore);

	LinkNode(RootNode, CopyTo);
	LinkNode(CopyTo, CopyFrom);
	LinkNode(CopyFrom, Output);
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