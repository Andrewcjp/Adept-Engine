
#include "EngineTypes.h"

bool TagContainer::Contains(Tag t)
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

void TagContainer::Add(std::string name)
{
	Tags.push_back(Tag(name));
}

bool Tag::operator==(const Tag& t)
{
	return t.name == name;
}
