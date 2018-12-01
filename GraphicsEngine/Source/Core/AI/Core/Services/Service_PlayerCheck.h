#pragma once
#include "ServiceBase.h"

struct BTValue;

class Service_PlayerCheck :public ServiceBase
{
public:
	CORE_API Service_PlayerCheck(BTValue* ValueTarget,BTValue* distanceValue = nullptr);
	~Service_PlayerCheck();
protected:
	virtual void Tick() override;
	BTValue* PlayerValue = nullptr;
	BTValue* DistanceValue = nullptr;
};

