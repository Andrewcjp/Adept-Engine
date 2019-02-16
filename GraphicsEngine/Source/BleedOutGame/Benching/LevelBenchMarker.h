#pragma once
struct BenchSettings
{
	BBTestMode::Type TestResolution = BBTestMode::HD;

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
	void Setup();
	void Init();
	void End();
	void Update();
	void Restart();
	GameObject* CameraObject = nullptr;
	float FlightSpeed = 10;
	float CompletionDistance = 0.75f;

private:
	int PreHeatFrames = 100;
	int CurrentSettingsIndex = 0;
	int PreHeat = 120;
	std::vector<SplineNodes> Points;
	std::vector<BenchSettings> Settings;
	int CurrnetPointIndex = 0;
	glm::vec3 Rot = glm::vec3(0, 0, 1);
};

