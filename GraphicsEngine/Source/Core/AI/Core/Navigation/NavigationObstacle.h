#pragma once
#include "Core/Components/Component.h"
class NavigationManager;
class NavigationObstacle : public Component
{
public:
	NavigationObstacle();
	~NavigationObstacle();
	void LinkToMesh(NavigationManager* parentmesh);
private:
	NavigationManager* ParentMesh = nullptr;
	

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void OnTransformUpdate();
};

