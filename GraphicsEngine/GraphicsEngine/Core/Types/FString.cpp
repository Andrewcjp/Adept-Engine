#include "Stdafx.h"
#include "FString.h"
#include "Core/Asserts.h"

FString::FString()
{}

FString::FString(std::string input)
{
	UnderlyingString = input;
}

FString::FString(const char * text)
{
	UnderlyingString = std::string(text);
}

FString::~FString()
{

}

bool FString::Contains(FString Target)
{
	if (UnderlyingString.empty())
	{
		return false;
	}
	if (UnderlyingString.find(Target.ToSString()) != -1)
	{
		return true;
	}
	return false;
}


void FString::RunFStringTests()
{
	FString test = FString("hello");
	ensure(test.Contains("h"))
	std::string data = "hello";
	test = FString(data);
	ensure(test.Contains("h"))
}
