#pragma once
class CollisionRect
{
public:
	CollisionRect()
	{}
	CollisionRect(int w, int h, int x, int y);
	~CollisionRect();
	bool Contains(int x, int y);

	int width;
	int height;
	int xoff;
	int yoff;
};

