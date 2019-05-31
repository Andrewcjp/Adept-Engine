#pragma once
#include "MeshPipelineController.h"
class MeshBatch;
class MeshDrawCommand;
class Shader;
struct MeshBatchElement;
class MeshBatchProcessor
{
public:
	MeshBatchProcessor();
	virtual ~MeshBatchProcessor();
	//Adds batches to this processor
	virtual void AddBatch(MeshBatch* Batch);
	//generates the mesh draw commands for this processor type
	//virtual void Process(MeshBatch* Batch);
	void Reset();
	void SubmitCommands(RHICommandList* List, Shader* shader = nullptr);
	virtual void OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command);
	void UpdateStats();
	virtual bool CheckProcess(MeshBatchElement* Element);
	bool IsSubInstance(MeshBatch* Batch);
protected:
	void AddDrawCommand(MeshDrawCommand* cmd);
	//Attempts to merge MeshDrawCommands present in this processor

	std::vector<MeshDrawCommand*> DrawCommands;
	void CountDrawCall();
	void Process(MeshBatch * Batch);
	ERenderPass::Type PassType = ERenderPass::Limit;
	bool DisableInstancing = false;
private:
	int DrawCallsThisFrame = 0;

};

