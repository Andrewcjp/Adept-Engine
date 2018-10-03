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
	virtual void ProcessSerialArchive(class Archive* A) override;
private:
	Camera* MCamera;
#if WITH_EDITOR
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props) override;
#endif
};

