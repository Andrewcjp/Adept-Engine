#pragma once
#include "Core/EngineTypes.h"

namespace ShaderPropertyType
{
	enum Type
	{
		Float,
		Float2,
		Float3,
		Float4,
		Bool,
		Int,		
	};
}

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
	std::string Name = "";
	ShaderPropertyType::Type Type = ShaderPropertyType::Float;
	std::string GetNameCode()
	{
		std::string output = "";
		if (IsDefined)
		{
			output = Name;
		}
		else
		{
			IsDefined = true;
			output = ShaderPropertyTypeToString(Type) + " " + Name;
		}
		return output;
	}
	void SetDefined(bool state)
	{
		IsDefined = state;
	}
private:
	bool IsDefined = false;
};
class ShaderGraphNode
{
public:
	ShaderGraphNode()
	{};
	virtual ~ShaderGraphNode()
	{};
	virtual std::string GetComplieCode() = 0;
	class ShaderGraph* Root = nullptr;
};

class SGN_MathNode : public ShaderGraphNode
{
public:
	SGN_MathNode()
	{};
	~SGN_MathNode()
	{};
	virtual std::string GetComplieCode()
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
	virtual std::string GetComplieCode()
	{
		std::stringstream Stream;
		Stream << TargetProp->GetNameCode() << " = " << "float3" << "(" << std::to_string(Value.x) << ", " << std::to_string(Value.y) << " ," << std::to_string(Value.z) << ");\n";
		return Stream.str();
	}
};

class CoreProps
{
public:
	PropertyLink * Diffusecolour = nullptr;
	PropertyLink * NormalDir = nullptr;
	CoreProps()
	{
		Diffusecolour = new PropertyLink();
		Diffusecolour->Name = "Diffuse";
		Diffusecolour->Type = ShaderPropertyType::Float3;

		NormalDir = new PropertyLink();
		NormalDir->Name = "Normal";
		NormalDir->SetDefined(true);
		NormalDir->Type = ShaderPropertyType::Float3;
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
	};
	TextureType::Type texType;
	~SGN_Texture()
	{};
	PropertyLink* TargetProp = nullptr;
	std::string Texname;

	virtual std::string GetComplieCode();
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
	virtual std::string GetComplieCode();
};