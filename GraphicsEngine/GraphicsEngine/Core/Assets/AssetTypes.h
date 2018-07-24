#pragma once
#include "EngineGlobals.h"
namespace AssetFileType
{
	enum Type
	{
		DDS,
		Image,//any format jpg, png etc.
		Mesh,
		Unknown,
		Count
	};
};

static AssetFileType::Type ParseAssetFileType(std::string Data);
struct AssetPathRef
{
	CORE_API AssetPathRef(std::string Filename);
	const std::string GetFullPathToAsset()const;
	const std::string GetNoExtPathToAsset() const;
	const std::string GetRelativePathToAsset()const;
	const std::string GetExtention()const;
	std::string Name;
	std::string BaseName;
	const AssetFileType::Type GetFileType() const;
private:
	std::string Extention;
	AssetFileType::Type FileType = AssetFileType::Unknown;
};
