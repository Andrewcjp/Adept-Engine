#pragma once
#include "TDShape.h"
namespace TD
{
	class TDAABB : public TDShape
	{
	public:
		TDAABB();
		TDAABB(glm::vec3 Pos,glm::vec3 size);
		~TDAABB();
		glm::vec3 GetMin()const;
		glm::vec3 GetMax()const;
		void DebugRender(glm::vec3 colour = glm::vec3(1), float time = 0.0f);

		glm::vec3 ClosestPoint(const glm::vec3 & point) const;

		static TDAABB * CreateFromMinMax(const glm::vec3 & min, const glm::vec3 & max);

		glm::vec3 HalfExtends = glm::vec3(size, size, size);
		glm::vec3 Position = glm::vec3(0,0,0);
		TDActor* Owner = nullptr;
		bool IsDead = false;
		bool IsPartOfAcceleration = true;//todo:
	private:		
		const float size = 10.0f;
	};
}

