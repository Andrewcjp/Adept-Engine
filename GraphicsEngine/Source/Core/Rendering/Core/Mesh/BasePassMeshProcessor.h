#pragma once
#include "Rendering\Core\Mesh\MeshBatchProcessor.h"
class BasePassMeshProcessor :public MeshBatchProcessor
{
public:
	BasePassMeshProcessor();
	~BasePassMeshProcessor();

	virtual void AddBatch(MeshBatch* Batch) override;
	virtual void Process(MeshBatch* Batch) override;
	virtual void SubmitCommands(RHICommandList* List, Shader* shader) override;
private:

};

