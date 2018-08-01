#pragma once
#include "Core/Assets/AssetTypes.h"
class ShaderGraph
{
public:
	ShaderGraph(FString Name);
	~ShaderGraph();
	void test();
	bool Complie(AssetPathRef Outputfile);
private:
	FString GraphName = "";
	std::vector<class ShaderGraphNode*> Nodes;

	class CoreProps* CoreGraphProperties = nullptr;
};

