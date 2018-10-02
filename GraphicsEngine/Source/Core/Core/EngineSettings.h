#pragma once
struct EngineSettings
{
	EngineSettings();
	bool Force2DOnly = false;
	float MaxPhysicsTimeStep = 1.0f / 30.0f;
};

