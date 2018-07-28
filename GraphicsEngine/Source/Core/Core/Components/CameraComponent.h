#pragma once
#include "Component.h"
class Camera;
class CameraComponent :
	public Component
{
public:
	CameraComponent();
	void InitComponent() override final;
	void SceneInitComponent() override final;
	~CameraComponent();

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	CORE_API static Camera* GetMainCamera();
private:
	Camera* MCamera;


	// Inherited via Component
	virtual void Serialise(rapidjson::Value & v) override;

	virtual void Deserialise(rapidjson::Value & v) override;

	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props) override;

};

