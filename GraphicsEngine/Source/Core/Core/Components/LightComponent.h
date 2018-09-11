#pragma once
#include "Component.h"
#include "Rendering/Core/Light.h"
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
	void SceneInitComponent() override final;
	void PostChangeProperties() override;
private:
	Light* MLight;
	float DefaultIntesity = 10.0;
	float intensity = 0;
	Light::LightType DefaultType = Light::Point;
	glm::vec3 DefaultColour = glm::vec3(1);
	bool DefaultShadow = false;
	bool Shadow = false;
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props) override;
	virtual void ProcessSerialArchive(class Archive* Arch) override;
};

