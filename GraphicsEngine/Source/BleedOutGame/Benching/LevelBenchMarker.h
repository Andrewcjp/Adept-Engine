#pragma once
struct BenchSettings
{
	BBTestMode::Type TestResolution = BBTestMode::HD;
	MGPUMode::Type TestType = MGPUMode::None;
};
class GameObject;
class LevelBenchMarker
{
public:
	struct SplineNodes
	{
		glm::vec3 Pos;
		glm::vec3 Forward;
	};
	LevelBenchMarker();
	~LevelBenchMarker();
	void AddAllRes(MGPUMode::Type mode);
	void Setup();
	void Init();
	void TransitionToSetting(BenchSettings * setting, int index);
	void End();
	void Update();
	void Restart();
	GameObject* CameraObject = nullptr;
	float FlightSpeed = 10;
	float CompletionDistance = 0.75f;

private:
	bool CloseOnFinish = false;
	BenchSettings* CurrnetSetting = nullptr;
	int CurrentSettingsIndex = 0;
	std::vector<SplineNodes> Points;
	std::vector<BenchSettings> Settings;
	int CurrnetPointIndex = 0;
	glm::vec3 Rot = glm::vec3(0, 0, 1);
	float PreHeatTimer = 0.0f;
	float PreHeatTime = 1.0f;
	bool Once = true;
};

