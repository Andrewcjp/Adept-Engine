
#include "TDTypes.h"

TD::TDPhysicalMaterial::TDPhysicalMaterial()
{
	Restitution = 0.2f;
}

void TD::ContactData::Reset()
{
	ContactPoints[MAX_CONTACT_POINTS_COUNT];
	Blocking = false;
	Direction[MAX_CONTACT_POINTS_COUNT];
	depth[MAX_CONTACT_POINTS_COUNT];
	ContactCount = 0;
}

void TD::TDFlagsBase::SetFlagValue(int flag, bool value)
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

bool TD::TDFlagsBase::GetFlagValue(int flag)
{
	return Flags & flag;
}

void TD::TDFlagsBase::SetFlags(int flags)
{
	Flags = Flags;
}
