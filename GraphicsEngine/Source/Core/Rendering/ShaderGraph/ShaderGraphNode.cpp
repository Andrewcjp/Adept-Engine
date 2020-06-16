
#include "ShaderGraphNode.h"
#include "ShaderGraph.h"

std::string SGN_Texture::GetComplieCode(ShaderGraph* context)
{
	context->AddTexDecleration("Texture2D " + Texname, Texname);
	std::stringstream Stream;
	Stream << TargetProp->GetNameCode(context) << " = ";
	if (texType == TextureType::Colour)
	{
		if (UseSFSFeedBack)
		{
			Stream << Texname << ".Sample(defaultSampler, input.uv, uint2(0,0),MipClamp.Sample(defaultSampler,input.uv)).rgb; \n";
		}
		else
		{
			Stream << Texname << ".Sample(defaultSampler, input.uv).rgb; \n";
		}
	}
	else if (texType == TextureType::Normal)
	{
		Stream << "(" << Texname << ".Sample(defaultSampler, input.uv).xyz)*2.0 - 1.0;\n Normal = normalize(mul(Normal, input.TBN));\n";
	}
	if (UseSFSFeedBack)
	{
		Stream << "FEEDBACK_WRITE(g_feedback, " << Texname << ", defaultSampler, input.uv);\n";
	}
	return Stream.str();
}

std::string SGN_CodeSnippet::GetComplieCode(ShaderGraph* context)
{
	return CodeSnip + "\n";
}

PropertyLink::PropertyLink(const std::string& name, ShaderPropertyType::Type t, const std::string& ddefault)
	:DefaultValue(ddefault)
{
	Name = name;
	Type = t;
}

std::string PropertyLink::GetNameCode(ShaderGraph* context)
{
	std::string output = "";
	if (context->IsPropertyDefined(Name))
	{
		output = Name;
	}
	else
	{
		context->AddDefine(Name);
		output = ShaderPropertyTypeToString(Type) + " " + Name;
	}
	return output;
}

std::string PropertyLink::GenDefault(ShaderGraph * context)
{
	if (context->IsPropertyDefined(Name))
	{
		//dont set the default value if already in use
		return "";
	}
	return ShaderPropertyTypeToString(Type) + " " + Name + " = " + DefaultValue + ";";
}

std::string PropertyLink::GetForBuffer()
{
	return ShaderPropertyTypeToString(Type) + " " + Name + ";";
}

void PropertyLink::ExposeToShader(ShaderGraph* context)
{
	context->AddDefine(Name);
	Exposed = true;
	context->BufferProps.push_back(this);
}
