#include "ShaderPreProcessor.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Utils/FileUtils.h"
#include <fstream>
std::string ShaderPreProcessor::ForceIncludeData = "";

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

bool ShaderPreProcessor::CheckIncludeExists(const std::string& file)
{
	return FileUtils::File_ExistsTest(AssetManager::GetShaderPath() + file);
}

std::string ShaderPreProcessor::LoadShaderIncludeFile(std::string name, IncludeStack* Stack, uint32* LineCount)
{
	const char * includeText = "#include";
	const int MaxIncludeTreeLength = 10;
	if (Stack == nullptr)
	{
		Stack = new IncludeStack();
	}
	std::string file;
	Stack->Limit++;
	if (Stack->Limit > MaxIncludeTreeLength)
	{
		return file;
	}

	std::string pathname = name;
	if (Stack->Limit > 0)
	{
		pathname = AssetManager::GetShaderPath();
		pathname.append(name);
	}
	if (!FileUtils::File_ExistsTest(pathname))
	{
		pathname = AssetManager::GetShaderPath();
		pathname.append(Stack->Relative);
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
			if (Stack->Limit == 1 && file.length() == 0)
			{
				file.append(GetForceInclude());
			}
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
					std::string data = "";
					const std::string RelativeFilePath = RelativeStartPath + line;
					if (CheckIncludeExists(RelativeFilePath))
					{
						if (!Stack->HasSeenInclude(RelativeFilePath))
						{
							Stack->IncludeList.push_back(RelativeFilePath);
							data = LoadShaderIncludeFile(RelativeFilePath.c_str(), Stack, LineCount);
						}
					}
					if (data.length() == 0 && CheckIncludeExists(line))
					{
						if (!Stack->HasSeenInclude(line))
						{
							Stack->IncludeList.push_back(line);
							data = LoadShaderIncludeFile(line.c_str(), Stack, LineCount);
						}
					}
					file.append(data);
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
			if (LineCount != nullptr)
			{
				(*LineCount)++;
			}
			file.append(" \n");
		}
		myfile.close();
	}
	else
	{
		if (Stack->Limit > 1)
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
	std::string ShaderCSOPath = AssetManager::GetShaderCacheDir() + FullShaderName;
	//if the source is newer than the CSO recompile
	return PlatformApplication::CheckFileSrcNewer(ShaderSRCPath, ShaderCSOPath);
}

std::string ShaderPreProcessor::GetForceInclude()
{
	if (ForceIncludeData.length() == 0)
	{
		std::ifstream myfile(AssetManager::GetShaderPath() + "Core\\Core.hlsl");
		std::string str((std::istreambuf_iterator<char>(myfile)), std::istreambuf_iterator<char>());
		ForceIncludeData = str + "\n";
		myfile.close();
	}
	return ForceIncludeData;
}

bool ShaderPreProcessor::CheckCSOValid(std::string Name, const std::string & ShaderNameHash)
{
	if (!CompareCachedShaderBlobWithSRC(Name, ShaderNameHash))
	{
		return false;
	}
	std::string name = Name;
	name.append(".hlsl");


	IncludeStack Stack;
	LoadShaderIncludeFile(name, &Stack);
	std::string ShaderCSOPath = AssetManager::GetShaderCacheDir() + ShaderNameHash;
	for (int i = 0; i < Stack.IncludeList.size(); i++)
	{
		std::string FullPath = AssetManager::GetShaderPath() + Stack.IncludeList[i];
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

void ShaderPreProcessor::FindRootConstants(ShaderSourceFile * file)
{
	std::vector<std::string> lines = StringUtils::Split(file->Source, '\n');
	const std::string Token = "PUSHCONST";
	for (int i = 0; i < lines.size(); i++)
	{
		if (lines[i].find(Token) != -1)
		{
			if (lines[i].find("register") == -1)
			{
				continue;
			}
			std::vector<std::string> DataSplit = StringUtils::Split(lines[i], ' ');
			LogEnsure(DataSplit.size() > 3);
			if (DataSplit.size() > 3)
			{
				std::string safeString = DataSplit[2];
				StringUtils::RemoveChar(safeString, ":");
				file->RootConstants.push_back(safeString);
			}
		}
	}
}

bool ShaderPreProcessor::IncludeStack::HasSeenInclude(std::string include)
{
	//#todo Chcek for same file with diffrent path
	return VectorUtils::Contains(IncludeList, include);
}
