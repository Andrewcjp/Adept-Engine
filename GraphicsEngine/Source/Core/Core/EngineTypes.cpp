
#include "EngineTypes.h"

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
	if (value)
	{
		Flags |= flag;
	}
	else
	{
		Flags &= ~flag;
	}
}

bool BitFlagsBase::GetFlagValue(int flag)
{
	return Flags & flag;
}

void BitFlagsBase::SetFlags(int flags)
{
	Flags = flags;
}