#include "BaseAsset.h"
#include "Core/Assets/Archive.h"
#include "Core/Platform/PlatformCore.h"

BaseAsset::BaseAsset()
{}

BaseAsset::~BaseAsset()
{}

void BaseAsset::LoadAsset(const std::string& SourceFile)
{
	PathToSource = SourceFile;
	//#Asset Load the GUID from the Meta data!
	Archive* A = new Archive(GetMetaFileName(PathToSource));
	A->HandleArchiveBody("Asset data");
	ProcessAsset(A);
	SafeDelete(A);
}

void BaseAsset::GenerateNewAsset(const std::string& AssetSourcePath)
{
	//#Asset GUID size_t type?
	GUID = PlatformMisc::GenerateGUID();
	PathToSource = AssetSourcePath;
}

void BaseAsset::SaveAsset()
{
	//#Asset Meta files for editor 
	Archive* A = new Archive(GetMetaFileName(PathToSource), true);
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

std::string BaseAsset::GetMetaFileName(std::string path)
{
	return path +".Meta";
}
