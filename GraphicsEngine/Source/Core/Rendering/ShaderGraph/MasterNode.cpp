#include "Stdafx.h"
#include "MasterNode.h"


MasterNode::MasterNode()
{
	Diffusecolour = new PropertyLink();
	Diffusecolour->Name = "Diffuse";
	Diffusecolour->Type = ShaderPropertyType::Float3;
	Diffusecolour->DefaultValue = "float3(1,1,1)";
	InputProps.push_back(Diffusecolour);
	NormalDir = new PropertyLink();
	NormalDir->Name = "Normal";
	NormalDir->Type = ShaderPropertyType::Float3;
}


MasterNode::~MasterNode()
{}

std::string MasterNode::Complie(ShaderGraph* g)
{
	std::string output = ComplieDefaultProps(g);
	output += GetNodeBody(g);
	return output;
}

std::string MasterNode::GetNodeBody(ShaderGraph * g)
{
	return std::string();
}

std::string MasterNode::ComplieDefaultProps(ShaderGraph * g)
{
	std::string output = "";
	for (int i = 0; i < InputProps.size(); i++)
	{
		output += InputProps[i]->GenDefault(g);
	}
	return output;
}

std::string MasterNode::GetTemplateName(MaterialShaderComplieData & data)
{
	if (data.RenderPassUseage == EMaterialPassType::Deferred)
	{
		return "MaterialTemplate_DEF_W_fs.hlsl";
	}
	return "MaterialTemplate_FWD_fs.hlsl";
}

PropertyLink * MasterNode::GetProp(std::string name)
{
	for (int i = 0; i < InputProps.size(); i++)
	{
		if (InputProps[i]->Name == name)
		{
			return InputProps[i];
		}
	}
	return nullptr;
}

void MasterNode::AddInput(std::string name, ShaderPropertyType::Type type, std::string defaultValue)
{
	PropertyLink* RoughNessProp = new PropertyLink();
	RoughNessProp->Type = type;
	RoughNessProp->Name = name;
	RoughNessProp->DefaultValue = defaultValue;
	InputProps.push_back(RoughNessProp);
}
