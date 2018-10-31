#pragma once
#include "TDShape.h"
namespace TD
{
	class TDBox :public TDShape
	{
	public:
		TDBox();
		~TDBox();
		glm::vec3 HalfExtends = glm::vec3(1, 1, 1);
		virtual glm::vec3 GetBoundBoxHExtents() override;
	};

}
