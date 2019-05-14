#pragma once
#include "MeshPipelineController.h"
class MeshBatch;
class MeshDrawCommand;
class Shader;
class MeshBatchProcessor
{
public:
	MeshBatchProcessor();
	virtual ~MeshBatchProcessor();
	//Adds batches to this processor
	virtual void AddBatch(MeshBatch* Batch);
	//generates the mesh draw commands for this processor type
	virtual void Process(MeshBatch* Batch);
	void Reset();
	virtual void SubmitCommands(RHICommandList* List, Shader* shader = nullptr) = 0;
	void UpdateStats();
protected:
	void AddDrawCommand(MeshDrawCommand* cmd);
	//Attempts to merge MeshDrawCommands present in this processor
	void MergeCommands();
	std::vector<MeshDrawCommand*> DrawCommands;
	void CountDrawCall();
	ERenderPass::Type PassType = ERenderPass::Limit;
private:
	int DrawCallsThisFrame = 0;

};

