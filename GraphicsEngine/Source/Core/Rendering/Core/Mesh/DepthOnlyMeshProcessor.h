#pragma once
#include "MeshBatchProcessor.h"

class SceneRenderer;
class Shader;
class DepthOnlyMeshProcessor :public MeshBatchProcessor
{
public:
	DepthOnlyMeshProcessor();
	virtual ~DepthOnlyMeshProcessor();

	void Init();

	virtual void AddBatch(MeshBatch* Batch) override;


	virtual void Process(MeshBatch* Batch) override;

	virtual void SubmitCommands(RHICommandList* List,  Shader* shader) override;
private:

};

