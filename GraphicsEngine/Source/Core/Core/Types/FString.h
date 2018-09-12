#pragma once

class FString
{
public:
	FString();
	FString(std::string input,bool Hash = false);
	FString(const char* text, bool Hash = false);
	~FString();
	bool Contains(FString Target);	
	std::string& ToSString();
	static void RunFStringTests();
	
	std::wstring& ToWideString();
	bool operator==(const FString &b) const;
	void SetupHash();
private:
	static void RunPerfTests();
	bool WideCached = false;
	std::string UnderlyingString;
	std::wstring WideString;
	int64_t Hash = -1;
};

