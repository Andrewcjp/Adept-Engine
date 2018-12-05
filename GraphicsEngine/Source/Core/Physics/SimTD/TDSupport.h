#pragma once
#include "TDSimulationCallbacks.h"
class TDSupport :public TD::TDSimulationCallbacks
{
public:
	TDSupport();
	~TDSupport();

	virtual void OnContact(const ContactPair** Contacts, int numContacts) override;
	virtual void OnTrigger(const ContactPair** Contacts, int numContacts) override;

};

