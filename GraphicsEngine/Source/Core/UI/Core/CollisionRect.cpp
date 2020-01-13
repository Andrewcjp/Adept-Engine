#include "CollisionRect.h"
#include "UI\UIManager.h"
#include "Rendering\Core\DebugLineDrawer.h"
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

void CollisionRect::DebugRender()
{
	DebugLineDrawer::Get2()->AddLine(glm::vec3(xoff, yoff, 0), glm::vec3(xoff + width, yoff, 0), Colours::RED);
	DebugLineDrawer::Get2()->AddLine(glm::vec3(xoff + width, yoff, 0), glm::vec3(xoff + width, yoff + height, 0), Colours::RED);

	DebugLineDrawer::Get2()->AddLine(glm::vec3(xoff, yoff + height, 0), glm::vec3(xoff + width, yoff + height, 0), Colours::RED);
	DebugLineDrawer::Get2()->AddLine(glm::vec3(xoff, yoff, 0), glm::vec3(xoff, yoff + height, 0), Colours::RED);
}
