#include "Stdafx.h"
#include "AssetTypes.h"
#include "Core/Engine.h"
#include "Core/Utils/StringUtil.h"
AssetPathRef::AssetPathRef(std::string Filename)
{
	std::vector<std::string> split = StringUtils::Split(Filename, '.');
	if (split.size() == 2)
	{
		Name = split[0];
		Extention = split[1];
		FileType = ParseAssetFileType(Extention);
	}
	else
	{
		Name = Filename;
	}
	split = StringUtils::Split(Name, '\\');
	if (split.size() > 0)
	{
		BaseName = split[split.size() - 1];
	}
}

const std::string AssetPathRef::GetFullPathToAsset() const
{
	return Engine::GetRootDir() + Name + "." + Extention;
}

const std::string AssetPathRef::GetNoExtPathToAsset() const
{
	return Engine::GetRootDir() + Name;
}

const std::string AssetPathRef::GetRelativePathToAsset() const
{
	return Name + "." + Extention;
}

const std::string AssetPathRef::GetExtention() const
{
	return Extention;
}

const AssetFileType::Type AssetPathRef::GetFileType() const
{
	return FileType;
}

AssetFileType::Type ParseAssetFileType(std::string Data)
{
	if (Data.find("dds") != -1 || Data.find("DDS") != -1)
	{
		return AssetFileType::DDS;
	}
	if (Data.find("jpg") != -1 || Data.find("png") != -1)
	{
		return AssetFileType::Image;
	}
	if (Data.find("obj") != -1 || Data.find("fbx") != -1)
	{
		return AssetFileType::Mesh;
	}
	return AssetFileType::Unknown;
}
