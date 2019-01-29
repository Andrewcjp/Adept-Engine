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
	float CompltionDistance = 0.25f;

private:
	int PreHeat = 20;
	std::vector<SplineNodes> Points;
	int CurrnetPointIndex = 0;
};

