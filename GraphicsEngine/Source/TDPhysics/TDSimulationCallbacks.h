#pragma once
#include "TDShape.h"
namespace TD
{


	class TDSimulationCallbacks
	{
	public:
		TD_API TDSimulationCallbacks();
		TD_API ~TDSimulationCallbacks();
		TD_API virtual void OnContact(const ContactPair** Contacts, int numContacts);
		TD_API virtual void OnTrigger(const ContactPair** Contacts, int numContacts);
	};
};

