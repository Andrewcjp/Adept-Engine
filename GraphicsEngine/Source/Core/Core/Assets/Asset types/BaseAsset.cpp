#include "BaseAsset.h"
#include "Core/Assets/Archive.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Assets/Asset types/BaseAsset.generated.h"
BaseAsset::BaseAsset()
{
	CALL_CONSTRUCTOR();
}

BaseAsset::~BaseAsset()
{}

void BaseAsset::LoadAsset(const std::string& SourceFile)
{
	PathToSource = SourceFile;
	AssetPath = SourceFile;
	StringUtils::RemoveChar(AssetPath, AssetManager::GetContentPath());
	name = GetFilename(AssetPath.c_str());
	//#Asset Load the GUID from the Meta data!
	Archive* A = new Archive(NeedsMetaFile ? GetMetaFileName(GetPathToSource()) : GetPathToSource());
	A->HandleArchiveBody("Asset data");
	ProcessAsset(A);
	SafeDelete(A);
}

void BaseAsset::GenerateNewAsset(const std::string& AssetSourcePath)
{
	//#Asset GUID size_t type?
	GUID = PlatformMisc::GenerateGUID();
	PathToSource = AssetSourcePath;
	AssetPath = AssetSourcePath;
	StringUtils::RemoveChar(AssetPath, AssetManager::GetContentPath());
}

void BaseAsset::CreateNew(const std::string& aAssetPath)
{
	GUID = PlatformMisc::GenerateGUID();
	AssetPath = aAssetPath;
	PathToSource = AssetManager::GetContentPath() + aAssetPath;
}

void BaseAsset::SaveAsset()
{
	//#Asset Meta files for editor 
	Archive* A = new Archive(NeedsMetaFile ? GetMetaFileName(GetPathToSource()) : GetPathToSource(), true);
	A->HandleArchiveBody("Asset data");
	ProcessAsset(A);
	A->EndHeaderWrite("Asset data");
	A->Write();
	SafeDelete(A);
}

void BaseAsset::ProcessAsset(Archive* A)
{
	ArchiveProp(GUID);
	//#Asset save the Hash of the asset type 
	ProcessArchive(A);
}

int BaseAsset::GetGUID() const
{
	return GUID;
}

EAssetStatus::Type BaseAsset::GetState() const
{
	return CurrentState;
}

bool BaseAsset::IsValid() const
{
	return GUID != 0;
}

std::string BaseAsset::GetMetaFileName(const std::string& path)
{
	return path + ".Meta";
}

void BaseAsset::OnPropertyUpdate(ClassReflectionNode* Node)
{
	SetDirty();
}

void BaseAsset::ProcessArchive(Archive * a)
{}
