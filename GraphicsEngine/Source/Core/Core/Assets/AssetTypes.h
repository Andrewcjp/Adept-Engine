#pragma once

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
	RHI_API AssetPathRef(std::string Filename);
	RHI_API AssetPathRef() {};
	RHI_API const std::string GetFullPathToAsset()const;
	RHI_API const std::string GetNoExtPathToAsset() const;
	RHI_API const std::string GetRelativePathToAsset()const;
	RHI_API const std::string GetExtention()const;
	std::string Name;
	std::string BaseName;
	RHI_API const AssetFileType::Type GetFileType() const;
	bool IsDDC = false;
	std::string DDCPath;
private:
	std::string Extention;

	AssetFileType::Type FileType = AssetFileType::Unknown;
};
