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
	EPlatforms::Type TargetPlatform = EPlatforms::Windows;
private:
	std::string OutputPath = "\\Build";
	bool ShouldPack = true;
	bool ShouldComplie = false;
	bool ShouldStage = true;
	std::string BuildConfig = "";
	
};
#endif
 