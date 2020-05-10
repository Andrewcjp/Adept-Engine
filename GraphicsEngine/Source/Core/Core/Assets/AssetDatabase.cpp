#include "AssetDatabase.h"

AssetDatabase* AssetDatabase::Instance = nullptr;

AssetDatabase::AssetDatabase()
{}

AssetDatabase::~AssetDatabase()
{}

void AssetDatabase::Build()
{

}

BaseAsset* AssetDatabase::FindAssetByPath(const std::string& path)
{
	return nullptr;
}
