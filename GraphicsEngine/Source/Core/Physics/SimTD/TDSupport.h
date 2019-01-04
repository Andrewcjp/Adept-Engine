#pragma once
#if TDSIM_ENABLED
#include "TDSimulationCallbacks.h"
#include "TDQuerryFilter.h"
namespace TD
{
	class TDActor;
	struct RaycastData;
	class TDShape;
}
class TDSupport :public TD::TDSimulationCallbacks
{
public:
	TDSupport();
	~TDSupport();

	virtual void OnContact(const ContactPair** Contacts, int numContacts) override;
	virtual void OnTrigger(const ContactPair** Contacts, int numContacts) override;

};
class IgnoreFilter :public TD::TDQuerryFilter
{
public:

	virtual bool PreFilter(TD::TDActor* actor, TD::TDShape* Shape, const TD::RayCast* raydata) override;
	virtual bool PostFilter(TD::RaycastData* Hit, const TD::RayCast* raydata) override;
	std::vector<RigidBody*> IgnoreActors;
};
#endif

