#pragma once
#include "MeshBatchProcessor.h"

class SceneRenderer;
class Shader;
class ZPrePassMeshProcessor :public MeshBatchProcessor
{
public:
	ZPrePassMeshProcessor();
	virtual ~ZPrePassMeshProcessor();

	void Init();

	virtual void AddBatch(MeshBatch* Batch) override;
	virtual void OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command) override;

	virtual bool CheckProcess(MeshBatchElement* Element) override;

private:

};

