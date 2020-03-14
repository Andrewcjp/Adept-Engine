#include "RenderGraph.h"
#include "Nodes/DeferredLightingNode.h"
#include "Nodes/GBufferWriteNode.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "Nodes/ParticleSimulateNode.h"
#include "Nodes/UpdateAccelerationStructuresNode.h"
#include "Nodes/RayTraceReflectionsNode.h"
#include "RenderNode.h"
#include "Nodes/VoxelReflectionsNode.h"
#include "StoreNodes/ShadowAtlasStorageNode.h"
#include "StoreNodes/SceneDataNode.h"
#include "NodeLink.h"
#include "Nodes/ShadowMaskNode.h"
#include "Nodes/ZPrePassNode.h"
#include "StorageNodeFormats.h"
#include "Nodes/ShadowUpdateNode.h"
#include "Nodes/UpdateReflectionsNode.h"
#include "Nodes/ParticleRenderNode.h"
#include "Nodes/PostProcessNode.h"
#include "Nodes/SSAONode.h"
#include "Nodes/DebugUINode.h"
#include "Nodes/VisModeNode.h"
#include "Nodes/OutputToScreenNode.h"
#include "Nodes/VRXShadingRateNode.h"
#include "Nodes/ForwardRenderNode.h"
#include "Nodes/SubmitToHMDNode.h"
#include "StoreNodes/InterGPUStorageNode.h"
#include "Nodes/InterGPUCopyNode.h"
#include "RenderGraphPatchLibrary.h"
#include "Nodes/RTFilterNode.h"
#include "Nodes/Flow/VRLoopNode.h"
#include "Nodes/Flow/MultiGPULoopNode.h"
#include "RHI/DeviceContext.h"

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

	FrameBufferStorageNode* EdgeDetectImage = AddStoreNode(new FrameBufferStorageNode("Edge Image"));
	Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R16_FLOAT;
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.SimpleStartingState = EResourceState::RenderTarget;
	Desc.AllowUnorderedAccess = true;
	EdgeDetectImage->SetFrameBufferDesc(Desc);


	RG_PatchMarker* marker = FindMarker(EBuiltInRenderGraphPatch::VRX)->Markers[0];
	if (marker == nullptr)
	{
		return;
	}
	RenderNode* LightingNode = marker->ExecuteOut;
	RenderNode* pp = marker->OutputTargets[0];

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
		if (pp != nullptr)
		{
			pp->GetInput(1)->SetStore(VRXShadingRateImage);
		}
	}
	//RateNode->GetInput(1)->SetLink(marker->Inputs[0]);
	RateNode->GetInput(2)->SetLink(marker->Inputs[1]);
	RateNode->GetInput(1)->SetStore(EdgeDetectImage);
	GBufferWriteNode* gBuffernode = (GBufferWriteNode*)marker->OutputTargets[2];
	gBuffernode->GetInput(1)->SetStore(EdgeDetectImage);
	marker->OutputTargets[3]->GetInput(2)->SetStore(EdgeDetectImage);
}

void RenderGraph::CreateDefGraphWithRT()
{
	RequiresRT = true;
	GraphName += "(RT)";
	RG_PatchSet* Patch = FindMarker(EBuiltInRenderGraphPatch::RT_Reflections);
	//find nodes
	DeferredLightingNode* LightNode = RenderNode::NodeCast<DeferredLightingNode>(Patch->Markers[0]->ExecuteOut);

	ShadowAtlasStorageNode* ShadowDataNode = StorageNode::NodeCast<ShadowAtlasStorageNode>(GetNodesOfType(EStorageType::ShadowData)[0]);

	FrameBufferStorageNode* RTXBuffer = CreateRTXBuffer();
	RenderNode* UpdateProbesNode = FindFirstOf(ParticleSimulateNode::GetNodeName());
	UpdateAccelerationStructuresNode* UpdateAcceleration = new UpdateAccelerationStructuresNode();
	LinkNode(UpdateProbesNode, UpdateAcceleration);

	RayTraceReflectionsNode* RTNode = new RayTraceReflectionsNode();
	LinkNode(UpdateAcceleration, RTNode);

	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetLink(Patch->Markers[0]->Inputs[ERG_Patch_Reflections::In_GBuffer]);
	RTNode->GetInput(2)->SetStore(ShadowDataNode);

	LightNode->UseScreenSpaceReflection = true;
	LinkNode(RTNode, LightNode);
	LightNode->GetInput(4)->SetLink(RTNode->GetOutput(0));
}

void RenderGraph::CreateDefGraphWithRT_VOXEL()
{
	GraphName += "(Voxel)RT";
	RG_PatchSet* Patch = FindMarker(EBuiltInRenderGraphPatch::RT_Reflections);
	DeferredLightingNode* LightNode = RenderNode::NodeCast<DeferredLightingNode>(Patch->Markers[0]->ExecuteOut);
	ShadowAtlasStorageNode* ShadowDataNode = StorageNode::NodeCast<ShadowAtlasStorageNode>(GetNodesOfType(EStorageType::ShadowData)[0]);

	FrameBufferStorageNode* RTXBuffer = CreateRTXBuffer();
	FrameBufferStorageNode* VXBuffer = CreateRTXBuffer();
	FrameBufferStorageNode* LastFrameBuffer = CreateRTXBuffer();
	FrameBufferStorageNode* SPPList = AddStoreNode(new FrameBufferStorageNode("SSP Buffer"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R16_UINT;
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.LinkToBackBufferScaleFactor = 1.0f;
	Desc.AllowUnorderedAccess = true;
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_UNORDERED_ACCESS;
	Desc.clearcolour = glm::vec4(0, 0, 0, 0);
	SPPList->SetFrameBufferDesc(Desc);



	RenderNode* UpdateProbesNode = Patch->Markers[0]->ExecuteIn;
	UpdateAccelerationStructuresNode* UpdateAcceleration = new UpdateAccelerationStructuresNode();
	LinkNode(UpdateProbesNode, UpdateAcceleration);

	VoxelReflectionsNode* VXNode = new VoxelReflectionsNode();
	LinkNode(UpdateAcceleration, VXNode);
	VXNode->GetInput(0)->SetStore(VXBuffer);
	VXNode->GetInput(1)->SetLink(Patch->Markers[0]->Inputs[ERG_Patch_Reflections::In_GBuffer]);
	VXNode->GetInput(2)->SetStore(ShadowDataNode);
	LightNode->UseScreenSpaceReflection = true;

	RayTraceReflectionsNode* RTNode = new RayTraceReflectionsNode();
	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetLink(Patch->Markers[0]->Inputs[ERG_Patch_Reflections::In_GBuffer]);
	RTNode->GetInput(2)->SetStore(ShadowDataNode);

	//tmp: RT node merge results
	RTFilterNode* FilterNode = new RTFilterNode();
	FilterNode->GetInput(0)->SetStore(RTXBuffer);
	FilterNode->GetInput(1)->SetStore(VXBuffer);
	FilterNode->GetInput(2)->SetStore(LastFrameBuffer);
	FilterNode->GetInput(3)->SetStore(SPPList);

	LightNode->UseScreenSpaceReflection = true;
	LinkNode(VXNode, RTNode);
	LinkNode(RTNode, FilterNode);
	LinkNode(FilterNode, LightNode);
	LightNode->GetInput(4)->SetLink(FilterNode->GetOutput(0));
}

void RenderGraph::CreateDefGraphWithVoxelRT()
{
	GraphName += "(Voxel)";
	RG_PatchSet* Patch = FindMarker(EBuiltInRenderGraphPatch::RT_Reflections);

	//find nodes
	DeferredLightingNode* LightNode = RenderNode::NodeCast<DeferredLightingNode>(Patch->Markers[0]->ExecuteOut);
	FrameBufferStorageNode* RTXBuffer = CreateRTXBuffer();

	RenderNode* UpdateProbesNode = Patch->Markers[0]->ExecuteIn;
	VoxelReflectionsNode* RTNode = new VoxelReflectionsNode();
	LinkNode(UpdateProbesNode, RTNode);
	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetLink(Patch->Markers[0]->Inputs[ERG_Patch_Reflections::In_GBuffer]);
	LightNode->UseScreenSpaceReflection = true;
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

	//VRS
	RG_PatchMarker* VRXMarker = new RG_PatchMarker();
	VRXMarker->ExecuteIn = ParticleSimNode;
	VRXMarker->ExecuteOut = LightNode;
	VRXMarker->Inputs.push_back(WriteNode->GetOutput(0));
	VRXMarker->Inputs.push_back(MaskNode->GetOutput(0));
	VRXMarker->OutputTargets.push_back(PPNode);
	VRXMarker->OutputTargets.push_back(LightNode);
	VRXMarker->OutputTargets.push_back(WriteNode);
	VRXMarker->OutputTargets.push_back(VisNode);
	RG_PatchSet* VRXset = new RG_PatchSet();
	VRXset->AddPatchMarker(VRXMarker, EBuiltInRenderGraphPatch::VRX);
	Markers.AddPatchSet(VRXset);

	//RTX + VX
	RG_PatchMarker* RTMarker = new RG_PatchMarker();
	RTMarker->ExecuteIn = ParticleSimNode;
	RTMarker->ExecuteOut = LightNode;
	RTMarker->Inputs.resize(ERG_Patch_Reflections::In_Limit);
	RTMarker->Inputs[ERG_Patch_Reflections::In_GBuffer] = WriteNode->GetOutput(0);
	RTMarker->Inputs[ERG_Patch_Reflections::In_ShadowData] = ShadowUpdate->GetOutput(0);

	RG_PatchMarker* UpdateMarker = new RG_PatchMarker();
	UpdateMarker->ExecuteIn = UpdateProbesNode;
	UpdateMarker->ExecuteOut = ParticleSimNode;
	RG_PatchSet* Rtset = new RG_PatchSet();

	Rtset->AddPatchMarker(RTMarker, EBuiltInRenderGraphPatch::RT_Reflections);
	Rtset->AddPatchMarker(UpdateMarker, EBuiltInRenderGraphPatch::RT_Reflections);
	Markers.AddPatchSet(Rtset);
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
	LinkNode(PP, Debug);
	Debug->GetInput(0)->SetLink(renderNode->GetOutput(0));
	LinkNode(Debug, Output);
	Output->GetInput(0)->SetLink(FWDNode->GetOutput(0));

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


	VRLoopNode* VrStart = new VRLoopNode();
	LinkNode(simNode, VrStart);
	VrStart->SetLoopBody([this,MainBuffer, SceneData, ShadowDataNode](RenderNode* first) {
		ZPrePassNode* PreZ = new ZPrePassNode();
		PreZ->GetInput(0)->SetStore(MainBuffer);
		LinkNode(first, PreZ);
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

		
		SubmitToHMDNode* SubNode = new SubmitToHMDNode();
		SubNode->GetInput(0)->SetLink(FWDNode->GetOutput(0));
		LinkNode(Debug, SubNode);
		return SubNode; 
	});
	//LinkNode(simNode, VrStart);
	//LinkNode(VrStart, PreZ);
	OutputToScreenNode* Output = new OutputToScreenNode();
	LinkNode(VrStart, Output);
	Output->GetInput(0)->SetStore(MainBuffer);
	/*VRBranchNode* VrEnd = new VRBranchNode();
	VrEnd->VrLoopBegin = VrStart;
	LinkNode(SubNode, VrEnd);
	LinkNode(VrEnd, Output);*/


	
}

void RenderGraph::CreateFallbackGraph()
{
	GraphName = "Fallback Renderer";
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.clearcolour = glm::vec4(0.5f, 0, 0.5f, 1);
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
	Desc.clearcolour = glm::vec4(1, 1, 1, 1);
	MainBuffer->SetFrameBufferDesc(Desc);
	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;
	OutputToScreenNode* Output = new OutputToScreenNode();
	DebugUINode* Debug = new DebugUINode();
	Debug->SetDevice(RHI::GetDeviceContext(0));
	RootNode = Debug;
	Debug->ClearBuffer = false;
	Output->GetInput(0)->SetLink(Debug->GetOutput(0));
	
	FrameBufferStorageNode* OtherGPUCopy = AddStoreNode(new FrameBufferStorageNode());
	OtherGPUCopy->SetDevice(RHI::GetDeviceContext(1));
	Desc.clearcolour = glm::vec4(1, 0, 0, 1);
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_COPY_DEST;
	OtherGPUCopy->SetFrameBufferDesc(Desc);
	Desc.clearcolour = glm::vec4(1, 1, 1, 1);
	FrameBufferStorageNode* MainGPU = AddStoreNode(new FrameBufferStorageNode());
	MainGPU->SetDevice(RHI::GetDeviceContext(0));
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_COPY_DEST;
	MainGPU->SetFrameBufferDesc(Desc);
	Debug->GetInput(0)->SetStore(MainGPU);

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

	DebugUINode* DebugOtherGPU = new DebugUINode();
	DebugOtherGPU->SetDevice(RHI::GetDeviceContext(1));
	DebugOtherGPU->GetInput(0)->SetStore(OtherGPUCopy);
	DebugOtherGPU->ClearBuffer = true;

	RootNode = DebugOtherGPU;

	Output->GetInput(0)->SetStore(MainGPU);
	LinkNode(DebugOtherGPU, CopyTo);
	LinkNode(CopyTo, CopyFrom);
	LinkNode(CopyFrom, Debug);
	LinkNode(Debug, Output);
}

void RenderGraph::CreateSFR()
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

	
	MultiGPULoopNode* Loop = new MultiGPULoopNode();
	RootNode = Loop;
	Loop->SetLoopBody([this, MainBuffer, SceneData](RenderNode* First,DeviceContext* device)
	{
		FrameBufferStorageNode* TargetBuffer = MainBuffer;
		if (device->GetDeviceIndex() != 0)
		{
			TargetBuffer = AddStoreNode(new FrameBufferStorageNode());
			RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
			Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
			Desc.AllowUnorderedAccess = true;
			TargetBuffer->SetDevice(device);
			TargetBuffer->SetFrameBufferDesc(Desc);
		}
		ShadowAtlasStorageNode* ShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
		ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
		ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
		ZPrePassNode* PreZ = new ZPrePassNode();
		if (First != nullptr)
		{
			LinkNode(First, ShadowUpdate);			
		}
		else
		{
			RootNode = ShadowUpdate;
		}
		LinkNode(ShadowUpdate, PreZ);
		PreZ->GetInput(0)->SetStore(TargetBuffer);
		ForwardRenderNode* FWDNode = new ForwardRenderNode();
		LinkNode(PreZ, FWDNode);
		FWDNode->UseLightCulling = false;
		FWDNode->UsePreZPass = false;
		FWDNode->UpdateSettings();
		FWDNode->GetInput(0)->SetStore(TargetBuffer);
		FWDNode->GetInput(1)->SetStore(SceneData);
		FWDNode->GetInput(2)->SetStore(ShadowDataNode);

		PostProcessNode* PP = new PostProcessNode();
		LinkNode(FWDNode, PP);
		PP->GetInput(0)->SetStore(TargetBuffer);
		return PP;
	});

	OutputToScreenNode* Output = new OutputToScreenNode();

	DebugUINode* Debug = new DebugUINode();
	LinkNode(Loop, Debug);
	Debug->GetInput(0)->SetStore(MainBuffer);
	LinkNode(Debug, Output);
	Output->GetInput(0)->SetStore(MainBuffer);
}
