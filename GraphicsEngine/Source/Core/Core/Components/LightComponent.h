#pragma once
#include "Component.h"
#include "Rendering/Core/Light.h"
UCLASS();
class LightComponent :
	public Component
{
public:
	CLASS_BODY();
	LightComponent();
	~LightComponent();
	void InitComponent() override final;
	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	void SetLightType(ELightType::Type type);
	void SetDistance(float amt);
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
	PROPERTY();
	float CurrentIntensity = 0.0f;
	ELightType::Type CurrentType = ELightType::Point;
	PROPERTY(Name = "Light Color");
	glm::vec3 CurrentColour = glm::vec3(1);
	PROPERTY();
	bool DoesShadow = false;
	PROPERTY();
	float Distance = 512;
#if WITH_EDITOR
	void GetInspectorProps(std::vector<InspectorProperyGroup>& props) override;
#endif
};

