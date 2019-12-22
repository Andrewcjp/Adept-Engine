#include "RenderNode.h"
#include "StorageNode.h"
#include "NodeLink.h"
#include "RenderGraph.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "StoreNodes/SceneDataNode.h"
#include "StoreNodes/ShadowAtlasStorageNode.h"
#include "RHI/RHI.h"
#include "Nodes/Flow/VRBranchNode.h"
#include "../Core/FrameBuffer.h"

RenderNode::RenderNode()
{}

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

ENodeQueueType::Type RenderNode::GetNodeQueueType() const
{
	return NodeEngineType;
}

RenderNode * RenderNode::GetNextNode() const
{
	return Next;
}

bool RenderNode::IsComputeNode() const
{
	return NodeEngineType == ENodeQueueType::Compute;
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
	NodeActive = val;
}

void RenderNode::FindVRContext()
{
	RenderNode* N = this;
	while (N != nullptr)
	{
		if (N->IsVrBranchNode)
		{
			VRBranchContext = NodeCast<VRBranchNode>(N);
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
	if (VRBranchContext != nullptr)
	{
		return VRBranchContext->GetCurrentEye();
	}
	return EEye::Left;
}

FrameBuffer * RenderNode::GetFrameBufferFromInput(int index)
{
	ensure(GetInput(index)->GetStoreTarget());
	ensure(GetInput(index)->GetStoreTarget()->StoreType == EStorageType::Framebuffer);
	FrameBufferStorageNode* Node = static_cast<FrameBufferStorageNode*>(GetInput(index)->GetStoreTarget());
	if (VRBranchContext != nullptr)
	{
		return Node->GetFramebuffer(VRBranchContext->GetCurrentEye());
	}
	return Node->GetFramebuffer();
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
}

void RenderNode::SetEndStates(RHICommandList * list)
{
	for (int i = 0; i < EndTransitions.size(); i++)
	{
		EndTransitions[i].Execute(list, this);
	}
}

void ResourceTransition::Execute(RHICommandList * list, RenderNode* rnode)
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
		buffer->SetResourceState(list, TargetState);
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
