#include "Stdafx.h"
#include "MeshBatchProcessor.h"
#include "MeshDrawCommand.h"
#include "Core\Performance\PerfManager.h"
static ConsoleVariable UseDynamicInstancing("UseDynamicInstancing", 0, ECVarType::ConsoleOnly);

MeshBatchProcessor::MeshBatchProcessor()
{
	PerfManager::Get()->AddTimer("DrawCalls", "Render");
}


MeshBatchProcessor::~MeshBatchProcessor()
{}

void MeshBatchProcessor::AddBatch(MeshBatch * Batch)
{
	//check batch is relevant!
	//then process 
}

void MeshBatchProcessor::Process(MeshBatch* batch)
{
	//build a draw command for this batch
}

void MeshBatchProcessor::Reset()
{
	DrawCommands.clear();
	DrawCallsThisFrame = 0;
}

void MeshBatchProcessor::UpdateStats()
{
	PerfManager::AddToCountTimer("DrawCalls", DrawCallsThisFrame);
}

void MeshBatchProcessor::AddDrawCommand(MeshDrawCommand * cmd)
{
	DrawCommands.push_back(cmd);
}

void MeshBatchProcessor::MergeCommands()
{
	if (!UseDynamicInstancing.GetBoolValue())
	{
		return;
	}
	//check all commands and merge were possible
	//same mesh same material
	//equal same draw call
}

void MeshBatchProcessor::CountDrawCall()
{
	DrawCallsThisFrame++;
}
