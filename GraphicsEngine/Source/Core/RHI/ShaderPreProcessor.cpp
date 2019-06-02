#include "Stdafx.h"
#include "ShaderPreProcessor.h"


ShaderPreProcessor::ShaderPreProcessor()
{}


ShaderPreProcessor::~ShaderPreProcessor()
{}
void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}
void ShaderPreProcessor::ProcessForDef(std::string & data, ShaderProgramBase::Shader_Define* Def)
{
	//replaceAll(data, Def->Name, Def->Value);
	//replaceAll(data, "#ifdef " + Def->Name, Def->Value);
	//replaceAll(shaderData, "#if " + Def->Name, Def->Value);
	data.insert(0, "#define " + Def->Name + " " + Def->Value+"\n ");
}
bool ShaderPreProcessor::PreProcessDefines(std::vector<ShaderProgramBase::Shader_Define>& defines, std::string & shaderData)
{
	for (int i = 0; i < defines.size(); i++)
	{
		ShaderProgramBase::Shader_Define* Def = &defines[i];

		//shaderData.replace(Def->Name.c_str(), Def->Value.c_str());
		ProcessForDef(shaderData, Def);

	}
	return true;
}
