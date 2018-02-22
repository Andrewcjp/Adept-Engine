#pragma once
class Cooker
{
public:
	Cooker();
	~Cooker();
	void CopyToOutput();
	void CreatePackage();
private:
	std::string OutputPath = "\\Build\\";
};

