#pragma once
#include "../../InputController.h"
class SteamVRController : public InputController
{
public:
	SteamVRController();
	~SteamVRController();

	bool IsTrackingController() const override;

protected:
	void UpdateState() override;

};

