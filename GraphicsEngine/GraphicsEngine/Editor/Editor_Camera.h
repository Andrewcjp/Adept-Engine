#pragma once
#include "../Rendering/Core/Camera.h"
class Editor_Camera
{
public:
	Editor_Camera(Camera* cam = nullptr);
	~Editor_Camera();
	void Update(float t);
	bool GetEnabled() { return IsActive; }
	void SetEnabled(bool state) { IsActive = state; }
	Camera* GetCamera() { return MainCam; }
private:
	Camera* MainCam;
	float sensitvity = 0.5f;
	bool IsActive = false;
};

