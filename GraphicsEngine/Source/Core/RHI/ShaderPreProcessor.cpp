#include "ShaderPreProcessor.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Utils/FileUtils.h"
#include <fstream>
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
	data.insert(0, "#define " + Def->Name + " " + Def->Value + "\n ");
}
bool ShaderPreProcessor::PreProcessDefines(std::vector<ShaderProgramBase::Shader_Define>& defines, std::string & shaderData)
{
	for (int i = 0; i < defines.size(); i++)
	{
		ShaderProgramBase::Shader_Define* Def = &defines[i];
		ProcessForDef(shaderData, Def);
	}
	return true;
}

std::string ShaderPreProcessor::LoadShaderIncludeFile(std::string name, int limit, std::string Relative, std::vector<std::string> * IncludeList/* = nullptr*/)
{
	const char * includeText = "#include";
	const int MaxIncludeTreeLength = 10;
	std::string file;
	limit++;
	if (limit > MaxIncludeTreeLength)
	{
		return file;
	}
	std::string pathname = name;
	if (limit > 0)
	{
		pathname = AssetManager::GetShaderPath();
		pathname.append(name);
	}
	if (!FileUtils::File_ExistsTest(pathname))
	{
		pathname = AssetManager::GetShaderPath();
		pathname.append(Relative);
		pathname.append(name);
	}
	std::string RelativeStartPath = "";
	if (name.find('\\') != -1)
	{
		RelativeStartPath = name;
		std::vector<std::string> data = StringUtils::Split(name, '\\');
		StringUtils::RemoveChar(RelativeStartPath, data[data.size() - 1]);
	}

	std::ifstream myfile(pathname);
	if (myfile.is_open())
	{
		std::string line;
		while (std::getline(myfile, line))
		{
			if (line.find("#") != -1)
			{
				size_t targetnum = line.find(includeText);
				if (targetnum != -1)
				{
					if (line.find("//") != -1)//Contains a commented line 
					{
						file.append(" \n");
						continue;
					}
					line.erase(targetnum, includeLength);
					StringUtils::RemoveChar(line, "\"");
					StringUtils::RemoveChar(line, "\"");
					if (IncludeList != nullptr)
					{
						IncludeList->push_back(line);
						IncludeList->push_back(RelativeStartPath + line);
					}
					file.append(LoadShaderIncludeFile(line.c_str(), limit, RelativeStartPath));
				}
				else
				{
					file.append(line);
				}
			}
			else
			{
				file.append(line);
			}
			file.append(" \n");
		}
		myfile.close();
	}
	else
	{
		if (limit > 1)
		{
			Log::LogMessage("Failed to find include " + pathname, Log::Error);
		}
		else
		{
			Log::LogMessage("failed to load " + pathname, Log::Error);
		}
		return "";
	}
	return file;
}

bool ShaderPreProcessor::CompareCachedShaderBlobWithSRC(const std::string & ShaderName, const std::string & FullShaderName)
{
	std::string ShaderSRCPath = AssetManager::GetShaderPath() + ShaderName + ".hlsl";
	std::string ShaderCSOPath = AssetManager::GetDDCPath() + "Shaders\\" + FullShaderName;
	//if the source is newer than the CSO recompile
	return PlatformApplication::CheckFileSrcNewer(ShaderSRCPath, ShaderCSOPath);
}

bool ShaderPreProcessor::CheckCSOValid(std::string Name, std::string ShaderNameHash)
{
	if (!CompareCachedShaderBlobWithSRC(Name, ShaderNameHash))
	{
		return false;
	}
	std::string name = Name;
	name.append(".hlsl");

	std::vector<std::string> Includes;
	LoadShaderIncludeFile(name, 0, "", &Includes);
	std::string ShaderCSOPath = AssetManager::GetDDCPath() + "Shaders\\" + ShaderNameHash;
	for (int i = 0; i < Includes.size(); i++)
	{
		std::string FullPath = AssetManager::GetShaderPath() + Includes[i];
		if (!FileUtils::File_ExistsTest(FullPath))
		{
			continue;
		}
		if (PlatformApplication::CheckFileSrcNewer(ShaderCSOPath, FullPath))
		{
			return false;
		}
	}
	return true;
}
