#pragma once
///Class which handles everything todo with Ini config files
class IniHandler
{
public:
	IniHandler();
	~IniHandler();
	///Saves all console variables
	void SaveAllConfigProps();
	void LoadMainCFG();
private:
	std::string MainCFGfile = "";
};

