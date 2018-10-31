#pragma once

class SpawningPool : public Component
{
public:
	SpawningPool();
	~SpawningPool();

	// Inherited via Component
	virtual void InitComponent() override;
	void OnDestroy() override;
};

