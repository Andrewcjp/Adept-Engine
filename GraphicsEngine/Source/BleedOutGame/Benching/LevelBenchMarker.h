#pragma once

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
	void Init();
	void End();
	void Update();

	GameObject* CameraObject = nullptr;
	float FlightSpeed = 10;
	float CompletionDistance = 0.75f;

private:
	int PreHeat = 120;
	std::vector<SplineNodes> Points;
	int CurrnetPointIndex = 0;
};

