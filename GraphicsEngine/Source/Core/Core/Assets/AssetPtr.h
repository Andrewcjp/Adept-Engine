#pragma once
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
		m_Ptr = AssetDatabase::FindAssetByPath(AssetPath);
	}
private:
	void SetAssetDirect(T* asset)
	{
		m_Ptr = asset;
		AssetPath = asset->GetAssetPath();
	}
	friend class AssetDatabase;
	friend class UIPropertyField;
	T* m_Ptr = nullptr;
	std::string AssetPath = "";
	AssetPathRef PathRef;
};

