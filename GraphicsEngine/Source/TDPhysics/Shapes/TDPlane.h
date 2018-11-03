#pragma once
#include "TDShape.h"
namespace TD
{
	class TD_API TDPlane :public TDShape
	{
	public:
		TDPlane();
		~TDPlane();
		glm::vec3 GetNormal() const;
		virtual glm::vec3 GetBoundBoxHExtents() override;

	};
}

