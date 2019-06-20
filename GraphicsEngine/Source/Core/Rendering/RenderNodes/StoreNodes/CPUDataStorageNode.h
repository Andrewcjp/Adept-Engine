#pragma once
#include "../StorageNode.h"
//This node represents CPU Data inputs to the graph system such as Lights Meshes etc.
class CPUDataStorageNode : public StorageNode
{
public:
	CPUDataStorageNode();
	~CPUDataStorageNode();
};

