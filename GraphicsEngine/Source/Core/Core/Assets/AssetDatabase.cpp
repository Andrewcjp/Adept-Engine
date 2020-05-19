#include "AssetDatabase.h"
#ifdef PLATFORM_WINDOWS
#include <filesystem>
#endif
#include "AssetManager.h"
#include "Asset types/BaseAsset.h"
#include "Core/Utils/FileUtils.h"
#include "Core/Reflection/ObjectLibrary.h"
#include "Asset types/TextureAsset.h"
#include "Asset types/MeshAsset.h"
#include "Asset types/MaterialAsset.h"

AssetDatabase* AssetDatabase::Instance = nullptr;

AssetDatabase::AssetDatabase()
{
	Instance = this;
	RegisterAssetExtention("png", TextureAsset::TYPEID);	
	RegisterAssetExtention("obj", MeshAsset::TYPEID);
	RegisterAssetExtention("mat", MaterialAsset::TYPEID);
	Build();
}

AssetDatabase::~AssetDatabase()
{}

void AssetDatabase::Build()
{
	Log::LogMessage("Building Asset Database");
	std::vector<std::string> Files;

	for (auto& p : std::filesystem::recursive_directory_iterator(AssetManager::GetContentPath()))
	{
		if (!p.is_directory())
		{
			Files.push_back(p.path().string());
		}
	}

	for (int i = 0; i < Files.size(); i++)
	{
		BaseAsset* FoundAsset = CreateOrGetAsset(Files[i]);
		if (FoundAsset != nullptr)
		{
			AssetInstances.push_back(FoundAsset);
		}
	}
	Log::LogMessage("Finished Building Asset Database");
}

BaseAsset* AssetDatabase::FindAssetByPath(const std::string& path)
{
	for (int i = 0; i < AssetInstances.size(); i++)
	{
		if (AssetInstances[i]->GetAssetPath() == path)
		{
			return AssetInstances[i];
		}
	}
	return nullptr;
}

BaseAsset* AssetDatabase::CreateOrGetAsset(std::string path)
{
	BaseAsset* Asset = nullptr;
	if (FileUtils::File_ExistsTest(BaseAsset::GetMetaFileName(path)))
	{
		Asset = CreateAssetFromFile(path);
		if (Asset == nullptr)
		{
			return nullptr;
		}
		Asset->LoadAsset(path);
	}
	else
	{
		//#Asset create the correct derived class here.
		Asset = CreateAssetFromFile(path);
		if (Asset == nullptr)
		{
			return nullptr;
		}
		Asset->GenerateNewAsset(path);
		Asset->SaveAsset();
	}
	return Asset;
}

BaseAsset* AssetDatabase::CreateAssetFromFile(std::string filepath)
{
	AssetPathRef ref = AssetPathRef(filepath);
	auto it = ExtentionIdMap.find(ref.GetExtention());
	if (it != ExtentionIdMap.end())
	{
		return ObjectLibrary::Create<BaseAsset>(it->second);
	}
	return nullptr;
}

void AssetDatabase::RegisterAssetExtention(std::string name, uint64 TypeID)
{
	ExtentionIdMap.emplace(name, TypeID);
}

void AssetDatabase::GetAllAssetsOfType(ClassTypeID id, std::vector<BaseAsset*> & list)
{
	for (int i = 0; i < AssetInstances.size(); i++)
	{
		if (AssetInstances[i]->GetId() == id)
		{
			list.push_back(AssetInstances[i]);
		}
	}
}
