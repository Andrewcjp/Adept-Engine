#pragma once
#include "Component.h"
class GrassPatchComponent :
	public Component
{
public:
	GrassPatchComponent();
	~GrassPatchComponent();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;

	// Inherited via Component
	virtual void Serialise(rapidjson::Value & v) override;
	virtual void Deserialise(rapidjson::Value & v) override;
};

