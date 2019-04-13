#pragma once

class Archive;
//base class to handle ID, streaming and unloading of a resource
struct EAssetStatus
{
	enum Type
	{
		NotLoaded,
		Loaded,
		CurrentlyStreaming,
		EnqueuedForUnLoad,
	};
};
//#Asset Each asset should encode its type name into the meta data to create the correct version at the other end.
//#Asset Fast way to create assets of correct type.
class BaseAsset
{
public:
	//Blank assets can be created but are not valid
	BaseAsset();
	~BaseAsset();
	void LoadAsset(std::string SourceFile);
	void GenerateNewAsset(std::string AssetSourcePath);
	void SaveAsset();
	virtual void ProcessAsset(Archive* A);
	CORE_API int GetGUID() const;
	CORE_API EAssetStatus::Type GetState() const;
	//Has this asset been created?
	bool IsValid() const;
	static std::string GetMetaFileName(std::string path);
private:
	std::string PathToSource = "";
	int GUID = 0;
	EAssetStatus::Type CurrentState = EAssetStatus::NotLoaded;
};

