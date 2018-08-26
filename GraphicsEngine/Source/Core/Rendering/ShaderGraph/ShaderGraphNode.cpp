#include "Stdafx.h"
#include "ShaderGraphNode.h"
#include "ShaderGraph.h"

std::string SGN_Texture::GetComplieCode()
{
	Root->AddTexDecleration("Texture2D " + Texname, Texname);
	std::stringstream Stream;
	Stream << TargetProp->GetNameCode() << " = ";
	if (texType == TextureType::Colour)
	{
		Stream << Texname << ".Sample(g_sampler, input.uv).rgb; \n";
	}
	else if (texType == TextureType::Normal)
	{
		Stream << "(" << Texname << ".Sample(g_sampler, input.uv).xyz)*2.0 - 1.0;\n Normal = normalize(mul(Normal, input.TBN)); ";
	}
	return Stream.str();
}

std::string SGN_CodeSnippet::GetComplieCode()
{
	//Root->AddTexDecleration("Texture2D testexture ", Texname);
	//std::stringstream Stream;
	//Stream << TargetProp->GetNameCode() << " = " << "testexture.Sample(g_sampler, input.uv).rgb; \n";
	return CodeSnip + "\n";
}
