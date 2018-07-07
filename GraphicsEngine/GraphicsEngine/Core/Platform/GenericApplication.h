#pragma once
class GenericApplication
{
public:
	GenericApplication();
	~GenericApplication();
	bool ExecuteHostScript(std::string Filename, std::string Args);
};

