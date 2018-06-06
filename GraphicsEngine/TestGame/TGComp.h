#pragma once
#include "Core\Components\Component.h"
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

	// Inherited via Component
	virtual void Serialise(rapidjson::Value & v) override;
	virtual void Deserialise(rapidjson::Value & v) override;
};

