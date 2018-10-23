#pragma once
#include "Core/Components/Component.h"
class NavigationMesh;
class NavigationObstacle : public Component
{
public:
	NavigationObstacle();
	~NavigationObstacle();
	void LinkToMesh(NavigationMesh* parentmesh);
private:
	NavigationMesh* ParentMesh = nullptr;
	

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void OnTransformUpdate();
};

