#pragma once
#include "Core/Reflection/IReflect.h"

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
UCLASS()
class BaseAsset : public IReflect
{
public:
	CLASS_BODY_Reflect();
	//Blank assets can be created but are not valid
	BaseAsset();
	virtual ~BaseAsset();
	void LoadAsset(const std::string& SourceFile);
	void GenerateNewAsset(const std::string& AssetSourcePath);
	void SaveAsset();
	virtual void ProcessAsset(Archive* A);
	CORE_API int GetGUID() const;
	CORE_API EAssetStatus::Type GetState() const;
	//Has this asset been created?
	bool IsValid() const;
	static std::string GetMetaFileName(const std::string& path);

	std::string GetAssetPath() const { return AssetPath; }
	std::string GetName()const { return name; }
protected:
	bool NeedsMetaFile = true;
private:
	std::string name = "";
	std::string AssetPath = "";
	std::string PathToSource = "";
	int GUID = 0;
	EAssetStatus::Type CurrentState = EAssetStatus::NotLoaded;
};

