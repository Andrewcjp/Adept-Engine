#pragma once
#include "TDTypes.h"

namespace TD
{
	class TDShape;
	class TDActor;
	class TDQuerryFilter
	{
	public:
		TDQuerryFilter();
		~TDQuerryFilter();
		virtual bool PreFilter(TDActor* actor, TDShape* Shape, const RayCast* raydata) = 0;
		virtual bool PostFilter(RaycastData* Hit,const RayCast* raydata) = 0;
	};

}