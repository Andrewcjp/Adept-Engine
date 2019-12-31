#pragma once
#include "MeshBatchProcessor.h"
class VoxelPassMeshProcessor: public MeshBatchProcessor
{
public:


	void AddBatch(MeshBatch* Batch) override;
	void OnSubmitCommands(RHICommandList* List, MeshDrawCommand* Command, const MeshPassRenderArgs& args) override;

private:

};

