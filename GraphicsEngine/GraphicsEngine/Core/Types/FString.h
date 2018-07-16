#pragma once

class FString
{
public:
	FString();
	FString(std::string input);
	FString(const char* text);
	~FString();
	bool Contains(FString Target);	
	std::string& ToSString();
	static void RunFStringTests();
	std::wstring& ToWideString();
private:
	bool WideCached = false;
	std::string UnderlyingString;
	std::wstring WideString;
	
};

