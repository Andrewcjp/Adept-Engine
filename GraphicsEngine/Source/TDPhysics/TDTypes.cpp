
#include "TDTypes.h"

TD::TDPhysicalMaterial::TDPhysicalMaterial()
{
	Restitution = 0.2f;
}

void TD::ContactData::Reset()
{
	ContactPoints[MAX_CONTACT_POINTS_COUNT];
	Blocking = false;
	Direction = glm::vec3();
	depth = 0.0f;
	ContactCount = 0;
}
