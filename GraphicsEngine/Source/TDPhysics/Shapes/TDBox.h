#pragma once
#include "TDShape.h"
namespace TD
{
	class TDBox :public TDShape
	{
	public:
		TD_API TDBox();
		~TDBox();
		glm::vec3 HalfExtends = glm::vec3(1, 1, 1);
		glm::mat3 Rotation = glm::mat3();
		virtual glm::vec3 GetBoundBoxHExtents() override;
		glm::vec3 ClosestPoint(const glm::vec3 & point);
	};

}
