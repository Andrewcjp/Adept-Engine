#pragma once
class Cooker
{
public:
	Cooker();
	~Cooker();
	std::string GetTargetPath(bool AppendSlash = false);

	void CopyToOutput();
	void CookAllShaders();
	void CopyFolderToOutput(std::string Target, std::string PathFromBuild);
	bool CopyAssetToOutput(std::string RelTarget);
	void CreatePackage();
private:
	std::string OutputPath = "\\Build";
	bool ShouldPack = false;
	bool ShouldComplie = true;
	std::string BuildConfig = "";
};

