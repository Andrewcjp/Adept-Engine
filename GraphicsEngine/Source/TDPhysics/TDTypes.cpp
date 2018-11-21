
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
