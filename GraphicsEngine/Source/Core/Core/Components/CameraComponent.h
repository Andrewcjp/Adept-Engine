#pragma once
#include "Component.h"
class Camera;
UCLASS()
class CameraComponent :
	public Component
{
public:
	CLASS_BODY_Reflect();
	CORE_API CameraComponent();
	void InitComponent() override final;
	void SceneInitComponent() override final;
	~CameraComponent();

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	CORE_API static Camera* GetMainCamera();
	virtual void ProcessSerialArchive(class Archive* A) override;
	///hack!
	bool AllowRotSync = false;
private:
	PROPERTY(Name = "Field of View")
	float m_FOV = 90.0f;
	Camera* MCamera = nullptr;
#if WITH_EDITOR
	void GetInspectorProps(std::vector<InspectorProperyGroup>& props) override;
#endif
};

