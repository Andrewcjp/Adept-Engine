#pragma once
#include "TDShape.h"
namespace TD
{


	class TDSimulationCallbacks
	{
	public:
		TDSimulationCallbacks();
		~TDSimulationCallbacks();
		virtual void OnContact(const ContactPair* Contacts, int numContacts);
		virtual void OnTrigger(const ContactPair* Contacts, int numContacts);
	};
};

