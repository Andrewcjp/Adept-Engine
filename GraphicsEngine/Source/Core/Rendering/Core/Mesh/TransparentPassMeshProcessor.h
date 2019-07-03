#pragma once
#include "Rendering\Core\Mesh\MeshBatchProcessor.h"
class TransparentPassMeshProcessor :public MeshBatchProcessor
{
public:
	TransparentPassMeshProcessor();
	~TransparentPassMeshProcessor();

	virtual void AddBatch(MeshBatch* Batch) override;
	//virtual void Process(MeshBatch* Batch) override;
	//virtual void SubmitCommands(RHICommandList* List, Shader* shader) override;
	virtual void OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command, const MeshPassRenderArgs& args) override;

	virtual bool CheckProcess(MeshBatchElement* Element) override;

private:

};

