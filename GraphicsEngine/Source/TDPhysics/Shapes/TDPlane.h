#pragma once
#include "TDShape.h"
namespace TD
{
	class  TDPlane :public TDShape
	{
	public:
		TD_API TDPlane();;
		TD_API TDPlane(glm::vec3 Normal);
		TD_API ~TDPlane();
		TD_API glm::vec3 GetNormal() const;
		virtual glm::vec3 GetBoundBoxHExtents() override;

		glm::vec3 ClosestPoint(const glm::vec3& point);

		glm::vec3 Normal = glm::vec3(0,1,0);
		float PlaneDistance = 0.0f;
	};
}

