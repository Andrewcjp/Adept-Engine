#include "RenderNode.h"
#include "StorageNode.h"
#include "NodeLink.h"
#include "RenderGraph.h"
#include "StoreNodes/FrameBufferStorageNode.h"
#include "StoreNodes/SceneDataNode.h"
#include "StoreNodes/ShadowAtlasStorageNode.h"
#include "RHI/RHI.h"

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
	NextNode->LastNode = this;
}

NodeLink * RenderNode::GetInput(int index)
{
	return Inputs[index];
}

NodeLink * RenderNode::GetOutput(int index)
{
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

FrameBuffer * RenderNode::GetFrameBufferFromInput(int index)
{
	ensure(GetInput(index)->GetStoreTarget());
	ensure(GetInput(index)->GetStoreTarget()->StoreType == EStorageType::Framebuffer);
	return static_cast<FrameBufferStorageNode*>(GetInput(index)->GetStoreTarget())->GetFramebuffer();
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

void RenderNode::AddInput(EStorageType::Type TargetType, std::string format, std::string InputName)
{
	Inputs.push_back(new NodeLink(TargetType, format, InputName));
}

void RenderNode::AddOutput(EStorageType::Type TargetType, std::string format, std::string InputName)
{
	Outputs.push_back(new NodeLink(TargetType, format, InputName));
}

void RenderNode::AddOutput(NodeLink * Input, std::string format, std::string InputName)
{
	AddOutput(Input->TargetType, format, InputName);
}

void RenderNode::AddRefrence(EStorageType::Type TargetType, std::string format, std::string InputName)
{
	Refrences.push_back(new NodeLink(TargetType, format, InputName));
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