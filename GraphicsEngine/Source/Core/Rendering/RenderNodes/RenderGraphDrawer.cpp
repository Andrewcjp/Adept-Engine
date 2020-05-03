#include "Stdafx.h"
#include "RenderGraphDrawer.h"
#include "RenderGraph.h"
#include "RenderNode.h"
#include "..\Core\DebugLineDrawer.h"
#include "..\Renderers\TextRenderer.h"
#include "NodeLink.h"
#include "Core\Input\Input.h"
#include "Core\Assets\AssetManager.h"
#include "Core\Utils\FileUtils.h"
#include "StoreNodes\FrameBufferStorageNode.h"


RenderGraphDrawer::RenderGraphDrawer()
{}


RenderGraphDrawer::~RenderGraphDrawer()
{}

void RenderGraphDrawer::Draw(RenderGraph* G)
{
	return;
	CurrentGraph = G;
	drawer = DebugLineDrawer::Get2();
	RenderNode* itor = G->GetNodeAtIndex(0);
	glm::vec3 tPos = StartPos;
	nodesize = glm::vec3(200, -200, 0);
	int index = 0;
	while (itor->GetNextNode() != nullptr)
	{
		if (!itor->IsNodeActive())
		{
			itor = itor->GetNextNode();
		}
		DrawNode(itor, tPos, index);
		DrawLinks(itor, index, tPos);
		index++;
		tPos += glm::vec3(nodesize.x + NodeSpaceing, 0, 0);
		itor = itor->GetNextNode();
	}
}
void RenderGraphDrawer::Update()
{
	const float movesize = 10;
	if (Input::GetKey(KeyCode::I))
	{
		StartPos.x -= movesize;
	}
	if (Input::GetKey(KeyCode::O))
	{
		StartPos.x += movesize;
	}
}
void RenderGraphDrawer::DrawNode(RenderNode * Node, glm::vec3 & LastPos, int index)
{
	glm::vec3 BoxSize = nodesize;
	glm::vec3 BoxPos = LastPos + glm::vec3(nodesize.x, 0, 0);

	BoxPos.y += 100 * (index % 3);

	drawer->AddLine(BoxPos, BoxPos + glm::vec3(BoxSize.x, 0, 0), Colour);
	drawer->AddLine(BoxPos + glm::vec3(BoxSize.x, 0, 0), BoxPos + glm::vec3(BoxSize.x, BoxSize.y, 0), Colour);
	drawer->AddLine(BoxPos + glm::vec3(BoxSize.x, BoxSize.y, 0), BoxPos + glm::vec3(0, BoxSize.y, 0), Colour);
	drawer->AddLine(BoxPos + glm::vec3(0, BoxSize.y, 0), BoxPos, Colour);
	drawer->AddLine(BoxPos + glm::vec3(0, -20, 0), BoxPos + glm::vec3(BoxSize.x, -20, 0), Colour);
	drawer->AddLine(BoxPos + glm::vec3(BoxSize.x / 2, -20, 0), BoxPos + glm::vec3(BoxSize.x / 2, BoxSize.y, 0), Colour);
	TextRenderer::instance->RenderText(Node->GetName(), BoxPos.x, BoxPos.y - 10.0f, 0.3f, Colour);
	for (uint i = 0; i < Node->GetNumInputs(); i++)
	{
		NodeLink* Link = Node->GetInput(i);
		TextRenderer::instance->RenderText(Link->GetLinkName(), BoxPos.x, BoxPos.y - 40 - (InputSpaceing*i), 0.3, Colour);
	}
	for (uint i = 0; i < Node->GetNumOutputs(); i++)
	{
		NodeLink* Link = Node->GetOutput(i);
		TextRenderer::instance->RenderText(Link->GetLinkName(), BoxPos.x + nodesize.x / 2, BoxPos.y - 40 - (InputSpaceing*i), 0.3, Colour);
	}
}

void RenderGraphDrawer::DrawLinks(RenderNode * A, int index, glm::vec3 & LastPos)
{
	glm::vec3 BoxPos = LastPos + glm::vec3(nodesize.x, 0, 0);

	for (uint i = 0; i < A->GetNumInputs(); i++)
	{
		NodeLink* Link = A->GetInput(i);
		if (Link->StoreLink != nullptr)
		{
			if (Link->StoreLink->OwnerNode != nullptr)
			{
				glm::vec3 InputPos = GetPosOfNodeindex(index);
				InputPos.y = InputPos.y - 40 - (InputSpaceing*i) + 5;
				int oindex = CurrentGraph->GetIndexOfNode(Link->StoreLink->OwnerNode);
				glm::vec3 othernode = GetPosOfNodeindex(oindex);

				for (int x = 0; x < Link->StoreLink->OwnerNode->GetNumOutputs(); x++)
				{
					NodeLink* OtherLink = Link->StoreLink->OwnerNode->GetOutput(x);
					if (OtherLink == Link->StoreLink)
					{
						othernode.y -= 40 - InputSpaceing * x - 5;
						othernode.x += nodesize.x;
						break;
					}
				}
				drawer->AddLine(InputPos, othernode, Colour);
			}
		}
	}
}

glm::vec3 RenderGraphDrawer::GetPosOfNodeindex(int index)
{
	glm::vec3 othernode = StartPos + glm::vec3(nodesize.x, 0, 0) + (glm::vec3(nodesize.x + NodeSpaceing, 0, 0)*(index));
	othernode.y += 100 * (index % 3);
	return othernode;
}

std::string RenderGraphDrawer::CreateLinksforNode(RenderNode* node)
{
	std::string out;
	int NextNode = CurrentGraph->GetIndexOfNode(node->GetNextNode());
	std::string NodeName = "node" + std::to_string(CurrentGraph->GetIndexOfNode(node));
	std::string EndName = "node" + std::to_string(NextNode);
	if (NextNode != -1)
	{
		out += "\"" + NodeName + "\":f0 -> \"" + EndName + "\":f0 [	id = " + std::to_string(LinkId) + "	];\n";
		LinkId++;
	}
	for (int i = 0; i < node->GetNumInputs(); i++)
	{
		NodeLink* Inputlink = node->GetInput(i);
		if (Inputlink->StoreLink != nullptr)
		{
			EndName = "node" + std::to_string(CurrentGraph->GetIndexOfNode(Inputlink->StoreLink->OwnerNode));
			out += "\"" + EndName + "\":\"" + Inputlink->StoreLink->GetLinkName() + "\" -> \"" + NodeName + "\":\"" + Inputlink->GetLinkName() + +"\" [	id = " + std::to_string(LinkId) + "	];\n";
		}
		else if (Inputlink->GetStoreTarget() != nullptr)
		{
			std::string Typename = "";
			switch (Inputlink->GetStoreTarget()->StoreType)
			{
			case EStorageType::Framebuffer:
				Typename = "Framebuffer";
				break;
			case EStorageType::ShadowData:
				Typename = "ShadowData";
				break;
			case EStorageType::CPUData:
				Typename = "CPUData";
				break;
			case EStorageType::SceneData:
				Typename = "SceneData";
				break;
			}
			std::string nodename = "'" + Inputlink->GetStoreTarget()->Name + "' ";
			if (Inputlink->GetStoreTarget()->Name.length() == 0)
			{
				nodename = "";
			}
			std::string StoreItemName = Typename + ":" + Inputlink->GetStoreTarget()->DataFormat;
			if (Inputlink->GetStoreTarget()->StoreType == EStorageType::Framebuffer)
			{
				FrameBufferStorageNode* FBN = (FrameBufferStorageNode*)Inputlink->GetStoreTarget();
				RHIFrameBufferDesc Desc = FBN->GetFrameBufferDesc();
				std::string fomat = "RT:" + std::to_string(Desc.RenderTargetCount);
				fomat += "D:" + std::to_string(Desc.NeedsDepthStencil);
				fomat += "FMT:" + std::to_string(Desc.RTFormats[0]);
				StoreItemName = Typename + ":" + fomat;
			}
			out += "\"" + nodename+StoreItemName + "\" -> \"" + NodeName + "\":\"" + Inputlink->GetLinkName() + +"\" [	id = " + std::to_string(LinkId) + "	];\n";
		}
		LinkId++;
	}
	return out;
}

std::string GetLabelForNode(RenderNode* Node)
{
	std::string name = "" + Node->GetName();
	int count = Math::Max(Node->GetNumInputs(), Node->GetNumOutputs());
	for (uint i = 0; i < count; i++)
	{
		NodeLink* Inputlink = Node->GetInput(i);
		NodeLink* outlink = Node->GetOutput(i);
		name += "|{";
		if (Inputlink != nullptr)
		{
			name += "<" + Inputlink->GetLinkName() + ">" + Inputlink->GetLinkName() + "|";
		}
		if (outlink != nullptr)
		{
			name += "<" + outlink->GetLinkName() + ">" + outlink->GetLinkName();
		}
		name += "}";
	}

	return name;
}

void RenderGraphDrawer::WriteGraphViz(RenderGraph* G)
{
	CurrentGraph = G;
	std::string Data = "digraph g {	graph[	rankdir = \"LR\"]; \n";

	RenderNode* itor = G->GetNodeAtIndex(0);
	int index = 0;
	LinkId = 0;
	while (itor != nullptr)
	{
		//if (!itor->IsNodeActive())
		//{
		//	itor = itor->GetNextNode();
		//}
		Data += "\"node" + std::to_string(index) + "\" [	\n		label = \"<f0>" + GetLabelForNode(itor) + "\"	\n shape = \"record\"];\n ";
		//:f2
		Data += CreateLinksforNode(itor);
		index++;
		itor = itor->GetNextNode();
	}
	Data += "}";


	FileUtils::WriteToFile(AssetManager::GetGeneratedDir() + "\\Graph" + G->GetGraphName() + ".viz", Data);
	//Engine::RequestExit(0);
}