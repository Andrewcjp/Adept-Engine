#include "Source/TDPhysics/TDPCH.h"
#include "TDSimulationCallbacks.h"

namespace TD
{
	TDSimulationCallbacks::TDSimulationCallbacks()
	{}

	TDSimulationCallbacks::~TDSimulationCallbacks()
	{}

	void TDSimulationCallbacks::OnContact(const ContactPair** Contacts, int numContacts)
	{

	}

	void TDSimulationCallbacks::OnTrigger(const ContactPair** Contacts, int numContacts)
	{
		//__debugbreak();
	}
};