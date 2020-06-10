#pragma once
#if SUPPORTS_COOK

class Cooker
{
public:
	Cooker();
	~Cooker();
	std::string GetTargetPath(bool AppendSlash = false);

	void Execute();
	void CookAllShaders();
	void CopyFolderToOutput(std::string Target, std::string PathFromBuild);
	bool CopyAssetToOutput(std::string RelTarget);
	void CreatePackage();
	static void BuildAllMaterials();
	void BuildAll();
	void SetPlatform(EPlatforms::Type Platform);
	CORE_API EPlatforms::Type GetTargetPlatform()const;
	ERenderSystemType GetTargetRHI()const;
private:
	std::string OutputPath = "";
	bool ShouldPack = true;
	bool ShouldComplie = false;
	bool ShouldStage = true;
	std::string BuildConfig = "";
	EPlatforms::Type TargetPlatform = EPlatforms::Windows;
};

#endif
 