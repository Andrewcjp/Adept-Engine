#pragma once
#include "TDShape.h"
namespace TD 
{
	class TDSphere :public TDShape
	{
	public:
		TD_API TDSphere();
		 ~TDSphere();
		float Radius = 1;
		virtual glm::vec3 GetBoundBoxHExtents() override;

	};
}