#pragma once
#include "Component.h"
#include "../Rendering/Core/Light.h"
class LightComponent :
	public Component
{
public:
	LightComponent();
	~LightComponent();
	void InitComponent() override final;
	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	void SetLightType(Light::LightType type);
	void SetIntensity(float amt);
	void SetShadow(bool Shadow);
	void SetLightColour(glm::vec3 colour);
	Light* Internal_GetLightPtr();
	void OnTransformUpdate() override;
private:
	Light* MLight;
	float DefaultIntesity = 10.0;
	Light::LightType DefaultType = Light::Point;
	bool DefaultShadow = false;
	// Inherited via Component
	virtual void Serialise(rapidjson::Value & v) override;
	virtual void Deserialise( rapidjson::Value & v) override;
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props) override;
};

