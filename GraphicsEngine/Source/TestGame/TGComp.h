#pragma once
#include "Core\Components\Component.h"
//#pragma comment(lib, "GraphicsEngine.lib")
class TGComp :public Component
{
public:
	 TGComp();
	~TGComp();

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;

	// Inherited via Component
	virtual void InitComponent() override; 


};

