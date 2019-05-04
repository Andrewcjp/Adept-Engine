#include "Stdafx.h"
#include "MeshBatch.h"


MeshBatch::MeshBatch()
{}


MeshBatch::~MeshBatch()
{
	MemoryUtils::DeleteVector(elements);
}

void MeshBatch::AddMeshElement(MeshBatchElement * element)
{
	elements.push_back(element);
}
