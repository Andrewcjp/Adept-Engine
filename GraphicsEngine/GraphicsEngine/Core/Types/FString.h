#pragma once
class FString
{
public:
	FString();
	FString(std::string input);
	FString(const char* text);
	~FString();
	bool Contains(FString Target);
	
	std::string& ToSString()
	{
		return  UnderlyingString;
	}
	static void RunFStringTests();
private:
	std::string UnderlyingString;
	
};

