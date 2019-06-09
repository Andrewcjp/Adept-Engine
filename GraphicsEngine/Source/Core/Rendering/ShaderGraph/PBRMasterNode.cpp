#include "PBRMasterNode.h"


PBRMasterNode::PBRMasterNode()
{
	NormalDir = new PropertyLink();
	NormalDir->Name = "Normal";
	NormalDir->Type = ShaderPropertyType::Float3;

	AddInput("Diffuse", ShaderPropertyType::Float3, "float3(1,1,1)");
	Diffusecolour = InputProps[0];
	AddInput("Roughness", ShaderPropertyType::Float, "0.5f");
	AddInput("Metallic", ShaderPropertyType::Float, "0.0f");
}


PBRMasterNode::~PBRMasterNode()
{}
