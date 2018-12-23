
#include "TDBox.h"
namespace TD
{

	TDBox::TDBox()
	{
		ShapeType = TDShapeType::eBOX;
	}

	TDBox::~TDBox()
	{}

	glm::vec3 TDBox::GetBoundBoxHExtents()
	{
		return HalfExtends;
	}

	glm::vec3 TDBox::ClosestPoint(const glm::vec3& point)
	{
		glm::vec3 result = GetPos();
		glm::vec3 dir = point - GetPos();

		for (int i = 0; i < 3; ++i)
		{
			glm::mat3 mat = glm::mat3(GetTransfrom()->GetQuatRot());
			glm::vec3 axis = glm::vec3(mat[i][0], mat[i][1], mat[i][2]);
			float distance = glm::dot(dir, axis);

			if (distance > HalfExtends[i])
			{
				distance = HalfExtends[i];
			}
			if (distance < -HalfExtends[i])
			{
				distance = -HalfExtends[i];
			}

			result = result + (axis * distance);
		}

		return result;
	}
}