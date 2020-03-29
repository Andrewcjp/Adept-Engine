
#include "RenderNode.h"
#include "StorageNode.h"
#include "NodeLink.h"
#include "RenderGraph.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "StoreNodes/SceneDataNode.h"
#include "StoreNodes/ShadowAtlasStorageNode.h"
#include "RHI/RHI.h"

#include "../Core/FrameBuffer.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHICommandList.h"
#include "StoreNodes/BufferStorageNode.h"

RenderNode::RenderNode()
{
	Context = RHI::GetDefaultDevice();
}

RenderNode::~RenderNode()
{}

void RenderNode::UpdateSettings()
{
	Inputs.clear();
	Outputs.clear();
	Refrences.clear();
	OnNodeSettingChange();
}

void RenderNode::OnNodeSettingChange()
{}

void RenderNode::ExecuteNode()
{
	if (NodeControlsFlow)
	{
		OnExecute();
	}
	else
	{
		if (IsNodeActive())
		{
			OnExecute();
			ensure(HasRunBegin);
			ensure(HasRunEnd);
		}
		else
		{
			for (uint i = 0; i < Inputs.size(); i++)
			{
				if (i < GetNumOutputs())
				{
					PassNodeThough(i);
				}
			}
		}
		if (Next != nullptr)
		{
			Next->ExecuteNode();
		}
	}
}

void RenderNode::LinkToNode(RenderNode * NextNode)
{
	Next = NextNode;
	NextNode->SetLastNode(this);
}

NodeLink * RenderNode::GetInput(int index)
{
	if (index >= Inputs.size())
	{
		return nullptr;
	}
	return Inputs[index];
}

NodeLink * RenderNode::GetOutput(int index)
{
	if (index >= Outputs.size())
	{
		return nullptr;
	}
	return Outputs[index];
}

NodeLink * RenderNode::GetRefrence(int index)
{
	return Refrences[index];
}

uint RenderNode::GetNumInputs() const
{
	return (uint)Inputs.size();
}

uint RenderNode::GetNumOutputs() const
{
	return (uint)Outputs.size();
}

uint RenderNode::GetNumRefrences() const
{
	return (uint)Refrences.size();
}

EViewMode::Type RenderNode::GetViewMode() const
{
	return ViewMode;
}

ECommandListType::Type RenderNode::GetNodeQueueType() const
{
	return NodeEngineType;
}

DeviceContextQueue::Type RenderNode::GetNodeQueue() const
{
	if (NodeQueueType == DeviceContextQueue::Graphics)
	{
		if (NodeEngineType == ECommandListType::Compute || NodeEngineType == ECommandListType::RayTracing)
		{
			return DeviceContextQueue::Compute;
		}
	}
	return NodeQueueType;
}

RenderNode * RenderNode::GetNextNode() const
{
	return Next;
}

bool RenderNode::IsComputeNode() const
{
	return NodeEngineType == ECommandListType::Compute;
}

std::string RenderNode::GetName() const
{
	return "UNNAMED";
}

void RenderNode::ValidateNode(RenderGraph::ValidateArgs & args)
{
	for (NodeLink* NL : Inputs)
	{
		NL->Validate(args, this);
	}
	for (NodeLink* NL : Outputs)
	{
		NL->Validate(args, this);
	}
	for (NodeLink* NL : Refrences)
	{
		NL->Validate(args, this);
	}
}

void RenderNode::SetupNode()
{
	if (Context == nullptr)
	{
		Context = RHI::GetDefaultDevice();
	}
	RenderSettings S = *RHI::GetRenderSettings();
	if (IsNodeSupported(S))
	{
		OnSetupNode();
	}
	else
	{
		//this causes data to be passed though
		SetNodeActive(false);
	}
#if !BUILD_SHIPPING
	Log::LogMessage("Node " + GetName() + " has " + std::to_string(BeginTransitions.size()) + " and " + std::to_string(EndTransitions.size()) + " transitions");
#endif
}

bool RenderNode::IsNodeDeferred() const
{
	return IsNodeInDeferredMode;
}

void RenderNode::SetNodeDeferredMode(bool val)
{
	IsNodeInDeferredMode = val;
}

bool RenderNode::IsNodeActive() const
{
	return NodeActive;
}

void RenderNode::SetNodeActive(bool val)
{
	if (val != NodeActive && RHI::GetRenderSystem()->GetCurrentGraph() != nullptr)
	{
		//todo: issue if multiple graphs used
		RHI::GetRenderSystem()->GetCurrentGraph()->InvalidateGraph();
	}
	NodeActive = val;
}

void RenderNode::FindVRContext()
{
	RenderNode* N = this;
	while (N != nullptr)
	{
		if (N->IsVrBranchNode)
		{
			//			VRBranchContext = NodeCast<VRBranchNode>(N);
			break;
		}
		N = N->Next;
	}
}

bool RenderNode::IsBranchNode() const
{
	//not great
	return !IsVrBranchNode && NodeControlsFlow;
}
bool RenderNode::IsVRBranch() const
{
	return IsVrBranchNode;
}

bool RenderNode::IsNodeSupported(const RenderSettings& settings)
{
	return true;
}

void RenderNode::SetDevice(DeviceContext * D)
{
	Context = D;
}

NodeLink* RenderNode::GetInputLinkByName(const std::string& name)
{
	for (int i = 0; i < Inputs.size(); i++)
	{
		if (Inputs[i]->GetLinkName() == name)
		{
			return Inputs[i];
		}
	}
	return nullptr;
}

NodeLink* RenderNode::GetOutputLinkByName(const std::string& name)
{
	for (int i = 0; i < Outputs.size(); i++)
	{
		if (Outputs[i]->GetLinkName() == name)
		{
			return Outputs[i];
		}
	}
	return nullptr;
}

EEye::Type RenderNode::GetEye()
{
	return TargetEye;
}

FrameBuffer * RenderNode::GetFrameBufferFromInput(int index)
{
	ensure(GetInput(index)->GetStoreTarget());
	ensure(GetInput(index)->GetStoreTarget()->StoreType == EStorageType::Framebuffer);
	FrameBufferStorageNode* Node = static_cast<FrameBufferStorageNode*>(GetInput(index)->GetStoreTarget());
	return Node->GetFramebuffer(GetEye());
}
RHIBuffer* RenderNode::GetBufferFromInput(int index)
{
	ensure(GetInput(index)->GetStoreTarget());
	ensure(GetInput(index)->GetStoreTarget()->StoreType == EStorageType::Buffer);
	BufferStorageNode* Node = static_cast<BufferStorageNode*>(GetInput(index)->GetStoreTarget());
	return Node->GetBuffer();
}
ShadowAtlasStorageNode * RenderNode::GetShadowDataFromInput(int index)
{
	ensure(GetInput(index)->GetStoreTarget());
	ensure(GetInput(index)->GetStoreTarget()->StoreType == EStorageType::ShadowData);
	return static_cast<ShadowAtlasStorageNode*>(GetInput(index)->GetStoreTarget());
}

Scene * RenderNode::GetSceneDataFromInput(int index)
{
	ensure(GetInput(index)->GetStoreTarget());
	ensure(GetInput(index)->GetStoreTarget()->StoreType == EStorageType::SceneData);
	return static_cast<SceneDataNode*>(GetInput(index)->GetStoreTarget())->CurrnetScene;
}


void RenderNode::OnValidateNode(RenderGraph::ValidateArgs & args)
{

}
void RenderNode::AddResourceInput(EStorageType::Type TargetType, EResourceState::Type State, const std::string& format, const std::string& InputName)
{
	NodeLink* link = new NodeLink(TargetType, format, InputName, this);
	link->ResourceState = State;
	Inputs.push_back(link);
}

void RenderNode::AddResourceOutput(EStorageType::Type TargetType, EResourceState::Type State, const std::string& format, const std::string& InputName)
{
	NodeLink* link = new NodeLink(TargetType, format, InputName, this);
	link->ResourceState = State;
	Outputs.push_back(link);
}

void RenderNode::AddInput(EStorageType::Type TargetType, const std::string& format, const std::string& InputName)
{
	Inputs.push_back(new NodeLink(TargetType, format, InputName, this));
}

void RenderNode::AddOutput(EStorageType::Type TargetType, const std::string& format, const std::string& InputName)
{
	Outputs.push_back(new NodeLink(TargetType, format, InputName, this));
}

void RenderNode::AddOutput(NodeLink * Input, const std::string& format, const std::string& InputName)
{
	AddOutput(Input->TargetType, format, InputName);
}

void RenderNode::AddRefrence(EStorageType::Type TargetType, const std::string& format, const std::string&InputName)
{
	Refrences.push_back(new NodeLink(TargetType, format, InputName, this));
}
void RenderNode::LinkThough(int inputindex, int outputindex /*= -1*/)
{
	if (outputindex == -1)
	{
		outputindex = inputindex;
	}
	GetOutput(outputindex)->SetLink(GetInput(inputindex));
}
void RenderNode::PassNodeThough(int inputindex, std::string newformat /*= std::string()*/, int outputindex /*= -1*/)
{
	if (outputindex == -1)
	{
		outputindex = inputindex;
	}
	if (newformat.length() > 0)
	{
		GetInput(inputindex)->GetStoreTarget()->DataFormat = newformat;
	}
	GetOutput(outputindex)->SetStore(GetInput(inputindex)->GetStoreTarget());
}

void RenderNode::SetBeginStates(RHICommandList * list)
{
	for (int i = 0; i < BeginTransitions.size(); i++)
	{
		BeginTransitions[i].Execute(list, this);
	}
	HasRunBegin = true;
}

void RenderNode::SetEndStates(RHICommandList * list)
{
	for (int i = 0; i < EndTransitions.size(); i++)
	{
		EndTransitions[i].Execute(list, this);
	}
	HasRunEnd = true;
}

void ResourceTransition::Execute(RHICommandList * list, RenderNode* rnode)
{
	if (TransitonType == StateChange)
	{
		if (TargetState == EResourceState::Undefined || (Target == nullptr && StoreNode == nullptr))
		{
			return;
		}
		if (Target->TargetType == EStorageType::Framebuffer)
		{
			if (StoreNode == nullptr)
			{
				StoreNode = Target->GetStoreTarget();
			}
			FrameBufferStorageNode* Node = static_cast<FrameBufferStorageNode*>(StoreNode);
			if (Node == nullptr)
			{
				return;
			}
			FrameBuffer* buffer = Node->GetFramebuffer(rnode->GetEye());
			//Log::LogMessage("[Transition] " + rnode->GetName() + " from " + EResourceState::ToString(buffer->GetCurrentState()) + " to " + EResourceState::ToString(TargetState));
			buffer->SetResourceState(list, TargetState, false, TransitionMode);
		}
		else if (Target->TargetType == EStorageType::Buffer)
		{
			if (StoreNode == nullptr)
			{
				StoreNode = Target->GetStoreTarget();
			}
			BufferStorageNode* Node = static_cast<BufferStorageNode*>(StoreNode);
			if (Node == nullptr)
			{
				return;
			}
			RHIBuffer* buffer = Node->GetBuffer();
			//Log::LogMessage("[Transition] " + rnode->GetName() + " from " + EResourceState::ToString(buffer->GetCurrentState()) + " to " + EResourceState::ToString(TargetState));
			//buffer->SetResourceState(list, TargetState, false, TransitionMode);
			buffer->SetResourceState(list, TargetState);
		}
	}
	else if (TransitonType == QueueWait)
	{
		if (SignalingDevice == -1)
		{
			list->GetDevice()->InsertGPUWait(rnode->GetNodeQueue(), SignalingQueue);
		}
		else
		{
			list->GetDevice()->InsertCrossGPUWait(rnode->GetNodeQueue(),RHI::GetDeviceContext(SignalingDevice), SignalingQueue);
		}
	}
}

void RenderNode::AddBeginTransition(const ResourceTransition& transition)
{
	BeginTransitions.push_back(transition);
}

void RenderNode::AddEndTransition(const ResourceTransition& transition)
{
	EndTransitions.push_back(transition);
}

void RenderNode::OnResourceResize()
{}

int RenderNode::GetDeviceIndex() const
{
	return Context->GetDeviceIndex();
}

void RenderNode::OnGraphCreate()
{}

void RenderNode::SetTargetEye(EEye::Type eye)
{
	TargetEye = eye;
}
