#include "CollisionRect.h"
#include "UI\UIManager.h"
CollisionRect::CollisionRect(int w, int h, int x, int y)
{
	width = w;
	height = h;
	xoff = x; //((UIManager::instance->GetWidth()) - x);
	yoff = ((UIManager::instance->GetHeight()) - y);
}

CollisionRect::~CollisionRect()
{}

bool CollisionRect::Contains(int x, int y)
{
	if (x > (xoff) && x < (xoff + (width)))
	{
		if (y > (yoff - height) && y < (yoff))
		{
			return true;
		}
	}

	return false;
}
