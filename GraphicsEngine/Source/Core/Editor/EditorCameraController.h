#pragma once
//This is the one class that handles free look
class EditorCameraController
{
public:
	EditorCameraController();
	~EditorCameraController();
	void Update();

	Transform* Target = nullptr;
	float FastTranslateSpeed = 100;
	float BaseTranslateSpeed = 10;
	float LookSens = 0.1f;
private:
	float Rotx = 0;
	float RotY = 0;
};

