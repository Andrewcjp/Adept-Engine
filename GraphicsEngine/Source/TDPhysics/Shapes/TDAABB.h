#pragma once
#include "TDShape.h"
namespace TD
{
	class TDAABB
	{
	public:
		TDAABB();
		~TDAABB();
		glm::vec3 GetMin();
		glm::vec3 GetMax();
		void DebugRender();
		const float size = 10.0f;
		glm::vec3 HalfExtends = glm::vec3(size, size, size);
		glm::vec3 Position = glm::vec3(0,0,0);
		TDActor* Owner = nullptr;
	};
}

