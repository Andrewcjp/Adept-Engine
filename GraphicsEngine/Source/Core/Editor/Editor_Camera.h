#pragma once
#include "Rendering/Core/Camera.h"
#include "Core/Transform.h"

class VRCamera;
class EditorCameraController;
#if WITH_EDITOR
class Editor_Camera
{
public:
	Editor_Camera(Camera* cam = nullptr);
	~Editor_Camera();
	void Update(float t);
	bool GetEnabled()
	{
		return IsActive;
	}
	void SetEnabled(bool state)
	{
		IsActive = state;
	}
	Camera* GetCamera();
	//toggle between cams
	void SetVrCam(bool state);
	bool IsVRCam() const;
	EditorCameraController* Controller = nullptr;
private:
	Transform MainCamTransfrom;
	bool bIsVRCam = false;
	Camera* MainCam = nullptr;
	VRCamera* VRCam = nullptr;
	float sensitvity = 0.5f;
	bool IsActive = false;
};
#endif
