#pragma once
#include "MasterNode.h"

class PBRMasterNode :public MasterNode
{
public:
	PBRMasterNode();
	~PBRMasterNode();

	PropertyLink* RoughNessProp = nullptr;

};

