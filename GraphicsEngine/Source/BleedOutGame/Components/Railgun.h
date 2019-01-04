#pragma once
#include "Weapon.h"

class BleedOutPlayer;
class Railgun : public Weapon
{
public:
	Railgun(Scene* scene, BleedOutPlayer* cameraobj); ;
	~Railgun();
	virtual void Update(float delta) override;
	virtual void OnFire() override;
private:
	float timer = 0.0f;
	glm::vec3 vector = glm::vec3();
};

