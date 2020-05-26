#pragma once
#include "AssetDatabase.h"
template<class T>
class AssetPtr
{
public:
	AssetPtr()
	{}
	~AssetPtr()
	{}

	T* GetAsset() const { return m_Ptr; }
	bool IsValid()const { return m_Ptr != nullptr; }
	void SetAsset(std::string path)
	{
		AssetPath = path;
		if (path.length() > 0)
		{
			m_Ptr = (T*)AssetDatabase::Get()->FindAssetByPath(GetAssetPath());
		}
	}
	void SetAssetDirect(T* asset)
	{
		m_Ptr = asset;
		AssetPath = asset->GetAssetPath();
	}
	std::string GetAssetPath() const { return AssetPath; }
private:
	friend class AssetDatabase;
	friend class UIPropertyField;
	T* m_Ptr = nullptr;
	std::string AssetPath = "";
	AssetPathRef PathRef;
};

