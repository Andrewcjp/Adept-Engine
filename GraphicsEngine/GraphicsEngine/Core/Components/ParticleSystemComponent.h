#pragma once
#include "Component.h"
class ParticleSystemComponent :
	public Component
{
public:
	ParticleSystemComponent();
	~ParticleSystemComponent();

	// Inherited via Component
	virtual void InitComponent() override;
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	virtual void Serialise(rapidjson::Value & v) override;
	virtual void Deserialise(rapidjson::Value & v) override;
};

