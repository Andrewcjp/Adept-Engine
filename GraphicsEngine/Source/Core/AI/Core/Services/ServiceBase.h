#pragma once
#include "AI/Core/Behaviour/BehaviourTreeNode.h"
class ServiceBase
{
public:
	ServiceBase();
	~ServiceBase();
	void HandleTick();
	void HandleInit();
	virtual EBTNodeReturn::Type GetServiceStatus();
	std::string Name = "";
	BehaviourTree* Owner = nullptr;
protected:
	virtual void Tick();
	virtual void Init();
private:
	float UpdateRate = 0.1f;
	EBTNodeReturn::Type CurrentStatus = EBTNodeReturn::LIMIT;
};

