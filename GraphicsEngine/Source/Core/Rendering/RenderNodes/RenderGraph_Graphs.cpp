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
#include "Nodes/VelocityNode.h"
#include "Nodes/MGPU/CompressionNode.h"
#include "StoreNodes/BufferStorageNode.h"
#include "Nodes/UINode.h"
#include "Nodes/MGPU/ShadowMaskCompressionNode.h"
#include "Nodes/SimpleNode.h"
#include "Nodes/LightCullingNode.h"

void RenderGraph::ApplyEditorToGraph()
{
	GraphName += "(Editor)";
	OutputToScreenNode* OutputNode = (OutputToScreenNode*)FindFirstOf(OutputToScreenNode::GetNodeName());
	FrameBufferStorageNode* FinalCompostBuffer = AddStoreNode(new FrameBufferStorageNode("Editor Output"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.SimpleStartingState = EResourceState::PixelShader;
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


	RG_PatchSet* Patch = FindMarker(EBuiltInRenderGraphPatch::VRX);
	const VRXPatchData* marker = Patch->AccessMarker<VRXPatchData>(0);
	if (Patch == nullptr)
	{
		return;
	}
	RenderNode* LightingNode = marker->DeferredLightNode;
	RenderNode* pp = marker->PostNode;

	VRXShadingRateNode* RateNode = new VRXShadingRateNode();
	RateNode->GetInput(0)->SetStore(VRXShadingRateImage);
	LightingNode->GetLastNode()->LinkToNode(RateNode);
	RateNode->LinkToNode(LightingNode);
	NodeLink* link = LightingNode->GetInputLinkByName("VRX Image");
	if (link != nullptr)
	{
		link->SetStore(VRXShadingRateImage);
	}
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR())
	{
		if (pp != nullptr)
		{
			pp->GetInput(1)->SetStore(VRXShadingRateImage);
		}
	}
	RateNode->GetInput(2)->SetStore(marker->ShadowMask);
	RateNode->GetInput(1)->SetStore(EdgeDetectImage);

	marker->GBufferWrite->GetInput(1)->SetStore(EdgeDetectImage);
	marker->VizMode->GetInput(2)->SetStore(EdgeDetectImage);
}

void RenderGraph::CreateDefGraphWithRT()
{
	RequiresRT = true;
	GraphName += "(RT)";
	RG_PatchSet* Patch = FindMarker(EBuiltInRenderGraphPatch::RT_Reflections);
	const RelfectionsPatchData* marker = Patch->AccessMarker<RelfectionsPatchData>(0);
	//find nodes
	DeferredLightingNode* LightNode = marker->DeferredLightNode;



	FrameBufferStorageNode* RTXBuffer = CreateRTXBuffer();
	RenderNode* UpdateProbesNode = FindFirstOf(ParticleSimulateNode::GetNodeName());
	UpdateAccelerationStructuresNode* UpdateAcceleration = new UpdateAccelerationStructuresNode();
	LinkNode(UpdateProbesNode, UpdateAcceleration);

	RayTraceReflectionsNode* RTNode = new RayTraceReflectionsNode();
	LinkNode(UpdateAcceleration, RTNode);

	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetStore(marker->GBuffer);
	RTNode->GetInput(2)->SetStore(marker->ShadowData);

	LightNode->UseScreenSpaceReflection = true;
	LinkNode(RTNode, LightNode);
	LightNode->GetData().SSRData->SetStore(RTXBuffer);
}

void RenderGraph::CreateDefGraphWithRT_VOXEL()
{
	GraphName += "(Voxel)RT";
	RG_PatchSet* Patch = FindMarker(EBuiltInRenderGraphPatch::RT_Reflections);
	const RelfectionsPatchData* marker = Patch->AccessMarker<RelfectionsPatchData>(0);
	DeferredLightingNode* LightNode = RenderNode::NodeCast<DeferredLightingNode>(marker->DeferredLightNode);
	ShadowAtlasStorageNode* ShadowDataNode = marker->ShadowData;

	FrameBufferStorageNode* RTXBuffer = CreateRTXBuffer();
	FrameBufferStorageNode* VXBuffer = CreateRTXBuffer();
	FrameBufferStorageNode* LastFrameBuffer = CreateRTXBuffer();
	FrameBufferStorageNode* SPPList = AddStoreNode(new FrameBufferStorageNode("SSP Buffer"));
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R16_UINT;
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.LinkToBackBufferScaleFactor = 1.0f;
	Desc.AllowUnorderedAccess = true;
	Desc.SimpleStartingState = EResourceState::UAV;
	Desc.clearcolour = glm::vec4(0, 0, 0, 0);
	SPPList->SetFrameBufferDesc(Desc);

	RenderNode* UpdateProbesNode = Patch->Markers[0]->ExecuteIn;
	UpdateAccelerationStructuresNode* UpdateAcceleration = new UpdateAccelerationStructuresNode();
	LinkNode(UpdateProbesNode, UpdateAcceleration);

	VoxelReflectionsNode* VXNode = new VoxelReflectionsNode();
	LinkNode(UpdateAcceleration, VXNode);
	VXNode->GetInput(0)->SetStore(VXBuffer);
	VXNode->GetInput(1)->SetStore(marker->GBuffer);
	VXNode->GetInput(2)->SetStore(ShadowDataNode);
	LightNode->UseScreenSpaceReflection = true;

	RayTraceReflectionsNode* RTNode = new RayTraceReflectionsNode();
	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(1)->SetStore(marker->GBuffer);
	RTNode->GetInput(2)->SetStore(ShadowDataNode);

	//tmp: RT node merge results
	RTFilterNode* FilterNode = new RTFilterNode();
	FilterNode->GetInput(0)->SetStore(RTXBuffer);
	FilterNode->GetInput(1)->SetStore(VXBuffer);
	FilterNode->GetInput(2)->SetStore(LastFrameBuffer);
	FilterNode->GetInput(3)->SetStore(SPPList);
	FilterNode->GetInput(4)->SetStore(marker->VelocityBuffer);

	LightNode->UseScreenSpaceReflection = true;
	LinkNode(VXNode, RTNode);
	LinkNode(RTNode, FilterNode);
	LinkNode(FilterNode, LightNode);
	LightNode->GetData().SSRData->SetStore(RTXBuffer);
}

void RenderGraph::CreateDefGraphWithVoxelRT()
{
	GraphName += "(Voxel)";
	RG_PatchSet* Patch = FindMarker(EBuiltInRenderGraphPatch::RT_Reflections);
	const RelfectionsPatchData* marker = Patch->AccessMarker<RelfectionsPatchData>(0);
	ShadowAtlasStorageNode* ShadowDataNode = marker->ShadowData;
	//find nodes
	DeferredLightingNode* LightNode = marker->DeferredLightNode;
	FrameBufferStorageNode* RTXBuffer = CreateRTXBuffer();

	RenderNode* UpdateProbesNode = Patch->Markers[0]->ExecuteIn;
	VoxelReflectionsNode* RTNode = new VoxelReflectionsNode();
	LinkNode(UpdateProbesNode, RTNode);
	RTNode->GetInput(0)->SetStore(RTXBuffer);
	RTNode->GetInput(2)->SetStore(ShadowDataNode);
	RTNode->GetInput(1)->SetStore(marker->GBuffer);
	LightNode->UseScreenSpaceReflection = true;
	LinkNode(RTNode, LightNode);
	LightNode->GetData().SSRData->SetStore(RTXBuffer);

	VisModeNode* Vis = RenderNode::NodeCast<VisModeNode>(FindFirstOf(VisModeNode::GetNodeName()));
	if (Vis != nullptr)
	{
		Vis->GetInput(2)->SetStore(RTXBuffer);
	}
}
#define USE_VEL 1
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
	Desc.VarRateSettings.BufferMode = FrameBufferVariableRateSettings::VRR;
	Desc.AllowDynamicResize = true;
	MainBuffer->SetFrameBufferDesc(Desc);
	MainBuffer->SetRetained();



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
#if USE_VEL
	FrameBufferStorageNode* VelocityBuffer = AddStoreNode(new FrameBufferStorageNode("Velocity buffer"));
	Desc = RHIFrameBufferDesc::CreateColour(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.SimpleStartingState = EResourceState::RenderTarget;
	VelocityBuffer->SetFrameBufferDesc(Desc);
	VelocityNode* VelNode = new VelocityNode();
	VelNode->GetInput(0)->SetStore(VelocityBuffer);
	AddNode(VelNode);
#endif
	GBufferNode->StoreType = EStorageType::Framebuffer;
	GBufferNode->DataFormat = StorageFormats::DefaultFormat;
	GBufferWriteNode* WriteNode = new GBufferWriteNode();
#if USE_VEL
	AddNode(WriteNode);
#else
	AddNode(WriteNode);
#endif
	WriteNode->GetInput(0)->SetStore(GBufferNode);

	ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
	ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
	AddNode(ShadowUpdate);

	MaskNode->GetInput(0)->SetStore(ShadowMaskBuffer);
	MaskNode->GetInput(1)->SetStore(ShadowDataNode);
	MaskNode->GetInput(2)->SetStore(GBufferNode);
	AddNode(MaskNode);

	UpdateReflectionsNode* UpdateProbesNode = new UpdateReflectionsNode();
	UpdateProbesNode->GetInput(0)->SetStore(ShadowDataNode);
	AddNode(UpdateProbesNode);

	
	ParticleSimulateNode* ParticleSimNode = new ParticleSimulateNode();

	LightCullingNode* LightCull = new LightCullingNode();
	LightCull->GetData().DepthBuffer->SetStore(GBufferNode);

	AddNode(LightCull);
	AddNode(ParticleSimNode);
	DeferredLightingNode* LightNode = AddNode(new DeferredLightingNode());

	LightNode->GetData().GBuffer->SetStore(GBufferNode);
	LightNode->GetData().MainBuffer->SetStore(MainBuffer);
	LightNode->GetData().ShadowMaps->SetStore(ShadowDataNode);
	LightNode->GetData().ShadowMask->SetStore(ShadowMaskBuffer);
	LightCull->AddApplyToGraph(this,  GBufferNode, ShadowMaskBuffer, MainBuffer);
	LightNode->AddSkyBoxToGraph(this,  &LightNode->GetData());

	ParticleRenderNode* PRenderNode = new ParticleRenderNode();
	AddNode(PRenderNode);

	PRenderNode->GetInput(0)->SetStore(MainBuffer);
	PRenderNode->GetInput(1)->SetStore(GBufferNode);

	SSAONode* SSAO = new SSAONode();
	SSAO->GetInput(0)->SetStore(MainBuffer);
	SSAO->GetInput(1)->SetStore(GBufferNode);
	AddNode(SSAO);

	PostProcessNode* PPNode = new PostProcessNode();
	AddNode(PPNode);
	PPNode->GetInput(0)->SetStore(MainBuffer);

	DebugUINode* Debug = new DebugUINode();
	AddNode(Debug);
	Debug->GetInput(0)->SetStore(MainBuffer);

	VisModeNode* VisNode = new VisModeNode();
	VisNode->GetInput(0)->SetStore(MainBuffer);
	VisNode->GetInput(1)->SetStore(GBufferNode);
	AddNode(VisNode);

	OutputToScreenNode* Output = new OutputToScreenNode();
	AddNode(Output);
	Output->GetInput(0)->SetStore(MainBuffer);

	EndGraph(MainBuffer, Output);


	//VRS
	RG_PatchMark_NEW<VRXPatchData>* VRXMarker = new RG_PatchMark_NEW<VRXPatchData>();
	VRXMarker->ExecuteIn = ParticleSimNode;
	VRXMarker->ExecuteOut = LightNode;
	VRXMarker->Data.DeferredLightNode = LightNode;
	VRXMarker->Data.GBufferWrite = WriteNode;
	VRXMarker->Data.PostNode = PPNode;
	VRXMarker->Data.VizMode = VisNode;
	VRXMarker->Data.ShadowData = ShadowDataNode;
	VRXMarker->Data.ShadowMask = ShadowMaskBuffer;
	Markers.AddSinglePatch(VRXMarker, EBuiltInRenderGraphPatch::VRX);

	//RTX + VX
	RG_PatchMark_NEW<RelfectionsPatchData>* RtMarker = new RG_PatchMark_NEW<RelfectionsPatchData>();
	RtMarker->ExecuteIn = ParticleSimNode;
	RtMarker->ExecuteOut = LightNode;
	RtMarker->Data.GBuffer = GBufferNode;
	RtMarker->Data.ShadowData = ShadowDataNode;
	RtMarker->Data.VelocityBuffer = VelocityBuffer;
	RtMarker->Data.DeferredLightNode = LightNode;
	RtMarker->Data.UpdateStructuresNodeTarget = UpdateProbesNode;
	Markers.AddSinglePatch(RtMarker, EBuiltInRenderGraphPatch::RT_Reflections);
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
	renderNode->GetInput(0)->SetStore(MainBuffer);
	LinkNode(FWDNode, renderNode);

	BufferStorageNode* CompressData = AddStoreNode(new BufferStorageNode());
	CompressData->FramebufferNode = MainBuffer;

	CompressionNode* Compress = new CompressionNode();
	Compress->SetCompressMode(true);
	Compress->GetInput(0)->SetStore(MainBuffer);
	Compress->GetInput(1)->SetStore(CompressData);
	LinkNode(renderNode, Compress);

	CompressionNode* DECompress = new CompressionNode();
	DECompress->SetCompressMode(false);
	DECompress->GetInput(0)->SetStore(MainBuffer);
	DECompress->GetInput(1)->SetStore(CompressData);
	LinkNode(Compress, DECompress);

	PostProcessNode* PP = new PostProcessNode();
	PP->GetInput(0)->SetStore(MainBuffer);
	LinkNode(DECompress, PP);

	OutputToScreenNode* Output = new OutputToScreenNode();

	DebugUINode* Debug = new DebugUINode();
	LinkNode(PP, Debug);
	Debug->GetInput(0)->SetStore(MainBuffer);
	LinkNode(Debug, Output);
	Output->GetInput(0)->SetStore(MainBuffer);
	EndGraph(MainBuffer, Output);

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
	VrStart->SetLoopBody([this, MainBuffer, SceneData, ShadowDataNode](RenderNode* first, bool Righteye)
	{
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
		Debug->GetInput(0)->SetStore(MainBuffer);


		SubmitToHMDNode* SubNode = new SubmitToHMDNode();
		SubNode->GetInput(0)->SetStore(MainBuffer);
		//SubNode->SetTargetEye(Righteye ? EEye::Right : EEye::Left);
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
	Output->GetInput(0)->SetStore(MainBuffer);
	EndGraph(MainBuffer, Output);
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

	FrameBufferStorageNode* OtherGPUCopy = AddStoreNode(new FrameBufferStorageNode());
	OtherGPUCopy->SetDevice(RHI::GetDeviceContext(1));
	Desc.clearcolour = glm::vec4(1, 0, 0, 1);
	Desc.SimpleStartingState = EResourceState::CopySrc;
	OtherGPUCopy->SetFrameBufferDesc(Desc);
	Desc.clearcolour = glm::vec4(1, 1, 1, 1);
	FrameBufferStorageNode* MainGPU = AddStoreNode(new FrameBufferStorageNode());
	MainGPU->SetDevice(RHI::GetDeviceContext(0));
	Desc.SimpleStartingState = EResourceState::CopyDst;
	MainGPU->SetFrameBufferDesc(Desc);
	Debug->GetInput(0)->SetStore(MainGPU);
	Output->GetInput(0)->SetStore(MainGPU);

	InterGPUStorageNode* HostStore = AddStoreNode(new InterGPUStorageNode());
	HostStore->StoreTargets.push_back(OtherGPUCopy);

	InterGPUCopyNode* CopyTo = new InterGPUCopyNode(RHI::GetDeviceContext(1));
	CopyTo->Mode = InterGPUCopyNode::CopyToStage;
	CopyTo->GetInput(0)->SetStore(OtherGPUCopy);
	CopyTo->GetInput(1)->SetStore(HostStore);
	InterGPUCopyNode* CopyFrom = new InterGPUCopyNode(RHI::GetDeviceContext(0));
	CopyFrom->Mode = InterGPUCopyNode::CopyFromStage;

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

	EndGraph(MainBuffer, Output);
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
	InterGPUStorageNode* HostStore = AddStoreNode(new InterGPUStorageNode());
	//HostStore->StoreTargets.push_back(MainBuffer);

	MultiGPULoopNode* Loop = new MultiGPULoopNode();
	RootNode = Loop;
	Loop->SetLoopBody([this, MainBuffer, SceneData, HostStore](RenderNode* First, DeviceContext* device)
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
		ShadowDataNode->SetDevice(device);
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
		if (device->GetDeviceIndex() != 0)
		{
			BufferStorageNode* CompressData = AddStoreNode(new BufferStorageNode());
			CompressData->FramebufferNode = MainBuffer;
			CompressData->SetDevice(device);
			CompressionNode* Compress = new CompressionNode();
			Compress->SetCompressMode(true);
			Compress->GetInput(0)->SetStore(TargetBuffer);
			Compress->GetInput(1)->SetStore(CompressData);
			LinkNode(FWDNode, Compress);

			InterGPUCopyNode* Copy = new InterGPUCopyNode(device);
			Copy->Mode = InterGPUCopyNode::CopyToStage;
			Copy->GetInput(1)->SetStore(HostStore);
			Copy->GetInput(2)->SetStore(CompressData);
			LinkNode(Compress, Copy);
			return  (RenderNode*)Copy;
		}
		return (RenderNode*)FWDNode;
	});

	BufferStorageNode* CompressData = AddStoreNode(new BufferStorageNode());
	CompressData->FramebufferNode = MainBuffer;
	HostStore->BufferStoreTargets.push_back(CompressData);
	InterGPUCopyNode* Copy = new InterGPUCopyNode(RHI::GetDefaultDevice());
	Copy->Mode = InterGPUCopyNode::CopyFromStage;
	Copy->GetInput(2)->SetStore(CompressData);
	Copy->GetInput(1)->SetStore(HostStore);

	LinkNode(Loop, Copy);

	CompressionNode* DECompress = new CompressionNode();
	DECompress->SetCompressMode(false);
	DECompress->GetInput(0)->SetStore(MainBuffer);
	DECompress->GetInput(1)->SetStore(CompressData);
	LinkNode(Copy, DECompress);

	PostProcessNode* PP = new PostProcessNode();
	LinkNode(DECompress, PP);
	PP->GetInput(0)->SetStore(MainBuffer);
	OutputToScreenNode* Output = new OutputToScreenNode();

	DebugUINode* Debug = new DebugUINode();
	LinkNode(PP, Debug);
	Debug->GetInput(0)->SetStore(MainBuffer);
	LinkNode(Debug, Output);
	Output->GetInput(0)->SetStore(MainBuffer);
	EndGraph(MainBuffer, Output);
}

void RenderGraph::CreateMGPUShadows()
{
	GraphName = "Forward Renderer";
	SceneDataNode* SceneData = AddStoreNode(new SceneDataNode());
	FrameBufferStorageNode* MainBuffer = AddStoreNode(new FrameBufferStorageNode());
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(100, 100);
	Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	Desc.VarRateSettings.BufferMode = FrameBufferVariableRateSettings::VRR;
	Desc.AllowUnorderedAccess = true;
	MainBuffer->SetFrameBufferDesc(Desc);

	FrameBufferStorageNode* ShadowMask = AddStoreNode(new FrameBufferStorageNode());
	ShadowMask->SetFrameBufferDesc(Desc);
	FrameBufferStorageNode* Other_ShadowMask = AddStoreNode(new FrameBufferStorageNode());
	Other_ShadowMask->SetFrameBufferDesc(Desc);
	Other_ShadowMask->SetDevice(RHI::GetDeviceContext(1));

	InterGPUStorageNode* HostStore = AddStoreNode(new InterGPUStorageNode());


	MainBuffer->StoreType = EStorageType::Framebuffer;
	MainBuffer->DataFormat = StorageFormats::DefaultFormat;

	ShadowAtlasStorageNode* ShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
	ShadowUpdateNode* ShadowUpdate = new ShadowUpdateNode();
	RootNode = ShadowUpdate;
	ShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
	ParticleSimulateNode* simNode = new ParticleSimulateNode();
	if (RHI::GetDeviceCount() > 1)
	{
		ShadowAtlasStorageNode* OtherShadowDataNode = AddStoreNode(new ShadowAtlasStorageNode());
		ShadowUpdateNode* OtherShadowUpdate = new ShadowUpdateNode();
		OtherShadowUpdate->GetInput(0)->SetStore(ShadowDataNode);
		OtherShadowUpdate->SetDevice(RHI::GetDeviceContext(1));
		OtherShadowDataNode->SetDevice(RHI::GetDeviceContext(1));
		LinkNode(ShadowUpdate, OtherShadowUpdate);
		FrameBufferStorageNode* GBufferNode = AddStoreNode(new FrameBufferStorageNode("GBuffer"));
		RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateGBuffer(100, 100);
		Desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
		GBufferNode->SetFrameBufferDesc(Desc);
		GBufferNode->SetDevice(RHI::GetDeviceContext(1));
		GBufferWriteNode* GBuffer = new GBufferWriteNode();
		GBuffer->SetDevice(RHI::GetDeviceContext(1));
		GBuffer->GetInput(0)->SetStore(GBufferNode);
		LinkNode(OtherShadowUpdate, GBuffer);
		ShadowMaskNode* SM = new ShadowMaskNode();
		SM->SetDevice(RHI::GetDeviceContext(1));
		SM->GetInput(0)->SetStore(Other_ShadowMask);
		SM->GetInput(1)->SetStore(OtherShadowDataNode);
		SM->GetInput(2)->SetStore(GBufferNode);
		LinkNode(GBuffer, SM);
		LinkNode(SM, simNode);
	}
	else
	{
		LinkNode(ShadowUpdate, simNode);
	}

	ZPrePassNode* PreZ = new ZPrePassNode();
	ExposeItem(PreZ, StandardSettings::UsePreZ);

	LinkNode(simNode, PreZ);


	BufferStorageNode* CompressData = AddStoreNode(new BufferStorageNode());
	CompressData->FramebufferNode = MainBuffer;
	HostStore->BufferStoreTargets.push_back(CompressData);

	BufferStorageNode* Other_CompressData = AddStoreNode(new BufferStorageNode());
	Other_CompressData->SetDevice(RHI::GetDeviceContext(1));
	Other_CompressData->FramebufferNode = Other_ShadowMask;
	HostStore->BufferStoreTargets.push_back(CompressData);

	ShadowMaskCompressionNode* Compress = new ShadowMaskCompressionNode();
	Compress->SetCompressMode(true);
	Compress->SetDevice(RHI::GetDeviceContext(1));
	Compress->GetInput(0)->SetStore(Other_ShadowMask);
	Compress->GetInput(1)->SetStore(Other_CompressData);
	LinkNode(PreZ, Compress);

	InterGPUCopyNode* CopyTo = new InterGPUCopyNode(RHI::GetDeviceContext(1));
	CopyTo->Mode = InterGPUCopyNode::CopyToStage;
	CopyTo->GetInput(1)->SetStore(HostStore);
	CopyTo->GetInput(2)->SetStore(Other_CompressData);
	LinkNode(Compress, CopyTo);

	InterGPUCopyNode* CopyFrom = new InterGPUCopyNode(RHI::GetDeviceContext(0));
	CopyFrom->Mode = InterGPUCopyNode::CopyFromStage;
	CopyFrom->GetInput(1)->SetStore(HostStore);
	CopyFrom->GetInput(2)->SetStore(CompressData);
	LinkNode(CopyTo, CopyFrom);

	ShadowMaskCompressionNode* DECompress = new ShadowMaskCompressionNode();
	DECompress->SetCompressMode(false);
	DECompress->GetInput(0)->SetStore(ShadowMask);
	DECompress->GetInput(1)->SetStore(CompressData);
	LinkNode(CopyFrom, DECompress);
	PreZ->GetInput(0)->SetStore(MainBuffer);

	ForwardRenderNode* FWDNode = new ForwardRenderNode();
	LinkNode(DECompress, FWDNode);
	FWDNode->UseLightCulling = false;
	FWDNode->UsePreZPass = false;
	FWDNode->UpdateSettings();
	FWDNode->GetInput(0)->SetStore(MainBuffer);
	FWDNode->GetInput(1)->SetStore(SceneData);
	FWDNode->GetInput(2)->SetStore(ShadowDataNode);
	FWDNode->GetInput(3)->SetStore(ShadowMask);

	ParticleRenderNode* renderNode = new ParticleRenderNode();
	renderNode->GetInput(0)->SetStore(MainBuffer);
	LinkNode(FWDNode, renderNode);

	PostProcessNode* PP = new PostProcessNode();
	PP->GetInput(0)->SetStore(MainBuffer);
	LinkNode(renderNode, PP);

	OutputToScreenNode* Output = new OutputToScreenNode();

	DebugUINode* Debug = new DebugUINode();
	LinkNode(PP, Debug);
	Debug->GetInput(0)->SetStore(MainBuffer);
	LinkNode(Debug, Output);
	Output->GetInput(0)->SetStore(MainBuffer);
	EndGraph(MainBuffer, Output);
}
