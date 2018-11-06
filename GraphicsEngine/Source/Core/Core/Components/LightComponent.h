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
#if WITH_EDITOR
	void PostChangeProperties() override;
#endif
private:
	Light* MLight = nullptr;
	float CurrentIntensity = 0.0f;
	Light::LightType CurrentType = Light::Point;
	glm::vec3 CurrentColour = glm::vec3(1);
	bool DoesShadow = false;
#if WITH_EDITOR
	void GetInspectorProps(std::vector<InspectorProperyGroup>& props) override;
#endif
	virtual void ProcessSerialArchive(class Archive* Arch) override;
};

