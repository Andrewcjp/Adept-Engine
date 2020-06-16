#pragma once
#include "Core/EngineTypes.h"
#include "ShaderGraph.h"
#include "../Core/Mesh/MaterialTypes.h"


static std::string ShaderPropertyTypeToString(ShaderPropertyType::Type type)
{
	switch (type)
	{
		case ShaderPropertyType::Float:
			return "float";
		case ShaderPropertyType::Float3:
			return "float3";
	}
	return "";
}
class PropertyLink
{
public:
	PropertyLink()
	{}
	PropertyLink(const std::string& name, ShaderPropertyType::Type t, const std::string& DefaultValue = "");
	std::string Name = "";
	ShaderPropertyType::Type Type = ShaderPropertyType::Float;
	std::string GetNameCode(ShaderGraph* context);
	//called before the master node is complied to ensure all props are defaulted
	std::string GenDefault(ShaderGraph* context);
	std::string DefaultValue = "";
	std::string GetForBuffer();
	void ExposeToShader(ShaderGraph* context);
	bool Exposed = false;
private:
};
class ShaderGraphNode
{
public:
	ShaderGraphNode()
	{};
	virtual ~ShaderGraphNode()
	{};
	virtual std::string GetComplieCode(ShaderGraph* context) = 0;
	class ShaderGraph* Root = nullptr;
};

class SGN_MathNode : public ShaderGraphNode
{
public:
	SGN_MathNode()
	{};
	~SGN_MathNode()
	{};
	virtual std::string GetComplieCode(ShaderGraph* context)
	{
		return "";
	}
};
//todo: template
class SGN_Constant : public ShaderGraphNode
{
public:
	SGN_Constant(PropertyLink* Prop, glm::vec3  value)
	{
		Value = value;
		TargetProp = Prop;
	};
	~SGN_Constant()
	{};
	glm::vec3 Value = glm::vec3(0, 0, 0);
	PropertyLink* TargetProp = nullptr;
	virtual std::string GetComplieCode(ShaderGraph* context)
	{
		std::stringstream Stream;
		Stream << TargetProp->GetNameCode(context) << " = " << "float3" << "(" << std::to_string(Value.x) << ", " << std::to_string(Value.y) << " ," << std::to_string(Value.z) << ");\n";
		return Stream.str();
	}
};

namespace TextureType
{
	enum Type
	{
		Colour,
		Normal
	};
}

class SGN_Texture : public ShaderGraphNode
{
public:
	SGN_Texture(PropertyLink* Prop, std::string name, TextureType::Type Type = TextureType::Colour)
	{
		TargetProp = Prop;
		Texname = name;
		texType = Type;
		UseSFSFeedBack = true;
	};
	TextureType::Type texType;
	bool UseSFSFeedBack = false;
	~SGN_Texture()
	{};
	PropertyLink* TargetProp = nullptr;
	std::string Texname;

	virtual std::string GetComplieCode(ShaderGraph* context);
};

class SGN_CodeSnippet : public ShaderGraphNode
{
public:
	SGN_CodeSnippet(PropertyLink* Prop, std::string code)
	{
		TargetProp = Prop;
		CodeSnip = code;
	};
	~SGN_CodeSnippet()
	{};
	PropertyLink* TargetProp = nullptr;
	std::string CodeSnip;
	virtual std::string GetComplieCode(ShaderGraph* context);
};