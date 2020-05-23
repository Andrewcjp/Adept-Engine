
#include "EngineTypes.h"
#include "Utils\StringUtil.h"

bool TagContainer::Contains(const Tag& t)
{
	for (int i = 0; i < Tags.size(); i++)
	{
		if (Tags[i] == t)
		{
			return true;
		}
	}
	return false;
}

void TagContainer::Add(const std::string& name)
{
	Tags.push_back(Tag(name));
}

bool Tag::operator==(const Tag& t)
{
	return t.name == name;
}

void BitFlagsBase::SetFlagValue(int flag, bool value)
{
	flag = 1 << flag;
	if (value)
	{
		Flags |= flag;
	}
	else
	{
		Flags &= ~flag;
	}
}

bool BitFlagsBase::GetFlagValue(int flag) const
{
	flag = 1 << flag;
	return Flags & flag;
}

void BitFlagsBase::SetFlags(int flags)
{
	Flags = flags;
}

int BitFlagsBase::ConvertToFlags(int enumi)
{
	return 1 << enumi;
}

std::string EPlatforms::ToString(EPlatforms::Type type)
{
	switch (type)
	{
	case EPlatforms::Windows:
		return "Win64";
	case EPlatforms::Linux:
		return "Linux";
	case EPlatforms::Android:
		return "Android";
		STRING_ADD_PLATFORMS;
	}
	return "Unknown";
}

EPlatforms::Type EPlatforms::Parse(std::string name)
{
	std::string Lower = name;
	StringUtils::ToLower(Lower);
	if (Lower == "win64")
	{
		return EPlatforms::Windows;
	}
	else if (Lower == "android")
	{
		return EPlatforms::Android;
	}
	else if (Lower == "linux")
	{
		return EPlatforms::Linux;
	}	
	PARSE_ADD_PLATFORMS;
	return EPlatforms::Limit;
}
