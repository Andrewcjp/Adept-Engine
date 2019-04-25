#pragma once
#include "ServiceBase.h"
/*!  \addtogroup AI
* @{ */
struct BTValue;

class Service_PlayerCheck :public ServiceBase
{
public:
	CORE_API Service_PlayerCheck(BTValue* ValueTarget, BTValue* distanceValue = nullptr, BTValue* RayCheck = nullptr);
	virtual ~Service_PlayerCheck();
	float VisionDistance = 40.0f;
protected:
	virtual void Tick() override;
	BTValue* PlayerValue = nullptr;
	BTValue* DistanceValue = nullptr;
	BTValue* RayCheckValue = nullptr;
};

