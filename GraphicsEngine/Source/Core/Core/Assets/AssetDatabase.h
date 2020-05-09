#pragma once

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

private:
	std::vector<BaseAsset*> AssetInstances;
	static AssetDatabase* Instance;
};

