#ifndef FROMTEST
#include "Stdafx.h"
#endif
#include "FString.h"
#include "Core/Asserts.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Platform/Logger.h"
#define AUTO_HASH 0
FString::FString()
{
#if AUTO_HASH
	SetupHash();
#endif
}

FString::FString(std::string input, bool Hash /*= false*/) :FString()
{
	UnderlyingString = input;
	if (Hash)
	{
		SetupHash();
	}
}

FString::FString(const char * text, bool Hash /*= false*/) :FString()
{
	UnderlyingString = std::string(text);
	if (Hash)
	{
		SetupHash();
	}
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


std::string & FString::ToSString()
{
	return  UnderlyingString;
}


std::wstring & FString::ToWideString()
{
	if (!WideCached)
	{
		WideString = StringUtils::ConvertStringToWide(UnderlyingString);
	}
	return WideString;
}

bool FString::operator==(const FString & b) const
{
	if (Hash == -1 || b.Hash == -1)
	{
		return UnderlyingString == b.UnderlyingString;
	}
	return Hash == b.Hash;
}

void FString::SetupHash()
{
	if (Hash != -1)
	{
		return;
	}
	Hash = std::hash<std::string>{} (UnderlyingString);
}



#if RUNTESTS
void FString::RunFStringTests()
{
	FString test = FString("hello");
	ensure(test.Contains("h"));
	std::string data = "hello";
	test = FString(data);
	ensure(test.Contains("h"));
	FString HashS = FString("h", true);
	FString Hash2 = FString("h", true);
	ensure(HashS == Hash2);
	ensure(test == FString("hello"));
	//RunPerfTests();
}
//Perf Tests
void Test(int length, std::string A, std::string B)
{
	std::string longString = "";
	for (int i = 0; i < length; i++)
	{
		longString += ((char)i + rand());
	}
	const char * String = longString.c_str();
	const long Itor = (long)1e4;
	FString HashS = FString(String, true);
	FString Hash2 = FString(String, true);
	FString NOHashS = FString(String, false);
	FString NOHash2 = FString(String, false);
	bool t = false;
	{
		const std::string Name = (B + std::to_string(length));
		SCOPE_STARTUP_COUNTER(Name.c_str());
		for (long i = 0; i < Itor; i++)
		{
			t = (NOHashS == NOHash2);
		}
	}
	{
		const std::string Name = (A + std::to_string(length));
		SCOPE_STARTUP_COUNTER(Name.c_str());
		for (long i = 0; i < Itor; i++)
		{
			t = (HashS == Hash2);
		}
	}

}
void FString::RunPerfTests()
{
	Test(1000, "FString Hashed ", "FString String ");
}
#endif