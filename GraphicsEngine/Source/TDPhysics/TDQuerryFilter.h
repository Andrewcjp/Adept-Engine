#pragma once
#include "TDTypes.h"

namespace TD
{
	class TDShape;
	class TDActor;
	class TDQuerryFilter
	{
	public:
		TD_API TDQuerryFilter();
		TD_API ~TDQuerryFilter();
		TD_API virtual bool PreFilter(TDActor* actor, TDShape* Shape, const RayCast* raydata) = 0;
		TD_API virtual bool PostFilter(RaycastData* Hit,const RayCast* raydata) = 0;
	};

}