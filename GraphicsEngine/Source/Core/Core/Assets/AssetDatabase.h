#pragma once
#include <map>

class BaseAsset;
class AssetDatabase
{
public:
	AssetDatabase();
	~AssetDatabase();
	void Build();
	static AssetDatabase* Get()
	{
		return Instance;
	};
	BaseAsset* FindAssetByPath(const std::string& path);
	BaseAsset* CreateOrGetAsset(std::string path);
	BaseAsset* CreateAssetFromFile(std::string filepath);
	std::vector<BaseAsset*> AssetInstances;
	void RegisterAssetExtention(std::string name, uint64 TypeID);
	void GetAllAssetsOfType(ClassTypeID id,std::vector<BaseAsset*> & list);
private:
	std::map<std::string, uint64> ExtentionIdMap;
	static AssetDatabase* Instance;
};

