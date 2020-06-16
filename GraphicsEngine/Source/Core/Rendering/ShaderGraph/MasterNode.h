#pragma once
#include "ShaderGraphNode.h"
//this handles all outputs for a shader
class MasterNode
{
public:
	MasterNode();
	virtual ~MasterNode();
	PropertyLink * Diffusecolour = nullptr;
	PropertyLink * NormalDir = nullptr;
	std::string Complie(ShaderGraph* g);
	std::string ComplieDefaultProps(ShaderGraph* g);
	virtual	std::string GetTemplateName(MaterialShaderCompileData& data);
	PropertyLink* GetProp(std::string name);
protected:
	std::vector<PropertyLink*> InputProps;
	void AddInput(std::string name, ShaderPropertyType::Type type, std::string defaultValue);
};

