
#include "CullingAABB.h"
#include "..\DebugLineDrawer.h"

CullingAABB::CullingAABB()
{}

CullingAABB::CullingAABB(glm::vec3 Pos, glm::vec3 size)
{
	Position = Pos;
	HalfExtends = size;
	SetScale(glm::vec3(1, 1, 1));
}

CullingAABB::~CullingAABB()
{}

glm::vec3 CullingAABB::GetMin() const
{
	glm::vec3 point1 = Position + HalfExtends;
	glm::vec3 point2 = Position - HalfExtends;
	return glm::vec3(fminf(point1.x, point2.x), fminf(point1.y, point2.y), fminf(point1.z, point2.z));
}

glm::vec3 CullingAABB::GetMax() const
{
	glm::vec3 point1 = Position + HalfExtends;
	glm::vec3 point2 = Position - HalfExtends;
	return glm::vec3(glm::max(point1.x, point2.x), glm::max(point1.y, point2.y), glm::max(point1.z, point2.z));
}

void CullingAABB::DebugRender(glm::vec3 colour /*= glm::vec3(1)*/, float time /*= 0.0f*/)
{

	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(ScaledHalfExtends.x, ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(ScaledHalfExtends.x, -ScaledHalfExtends.y, ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(ScaledHalfExtends.x, -ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(-ScaledHalfExtends.x, -ScaledHalfExtends.y, ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(-ScaledHalfExtends.x, -ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(-ScaledHalfExtends.x, ScaledHalfExtends.y, ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(-ScaledHalfExtends.x, ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(ScaledHalfExtends.x, ScaledHalfExtends.y, ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(ScaledHalfExtends.x, ScaledHalfExtends.y, -ScaledHalfExtends.z), Position + glm::vec3(ScaledHalfExtends.x, -ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(ScaledHalfExtends.x, -ScaledHalfExtends.y, -ScaledHalfExtends.z), Position + glm::vec3(-ScaledHalfExtends.x, -ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(-ScaledHalfExtends.x, -ScaledHalfExtends.y, -ScaledHalfExtends.z), Position + glm::vec3(-ScaledHalfExtends.x, ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(-ScaledHalfExtends.x, ScaledHalfExtends.y, -ScaledHalfExtends.z), Position + glm::vec3(ScaledHalfExtends.x, ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(ScaledHalfExtends.x, ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(ScaledHalfExtends.x, ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(ScaledHalfExtends.x, -ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(ScaledHalfExtends.x, -ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(-ScaledHalfExtends.x, -ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(-ScaledHalfExtends.x, -ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
	DebugLineDrawer::Get()->AddLine(Position + glm::vec3(-ScaledHalfExtends.x, ScaledHalfExtends.y, ScaledHalfExtends.z), Position + glm::vec3(-ScaledHalfExtends.x, ScaledHalfExtends.y, -ScaledHalfExtends.z), colour, time);
}

glm::vec3 CullingAABB::ClosestPoint(const glm::vec3& point)const
{
	glm::vec3 result = point;
	const glm::vec3 min = GetMin();
	const glm::vec3 max = GetMax();

	result.x = (result.x < min.x) ? min.x : result.x;
	result.y = (result.y < min.y) ? min.y : result.y;
	result.z = (result.z < min.z) ? min.z : result.z;

	result.x = (result.x > max.x) ? max.x : result.x;
	result.y = (result.y > max.y) ? max.y : result.y;
	result.z = (result.z > max.z) ? max.z : result.z;

	return result;
}

CullingAABB* CullingAABB::CreateFromMinMax(const glm::vec3 & min, const glm::vec3 & max)
{
	CullingAABB* r = new CullingAABB();
	r->HalfExtends = (max - min) * 0.5f;
	r->Position = (min + max) * 0.5f;
	r->SetScale(glm::vec3(1, 1, 1));
	return r;
}

void CullingAABB::AddAABB(CullingAABB* other)
{
	glm::vec3 max = glm::max(Position + HalfExtends, other->Position + other->HalfExtends);
	glm::vec3 min = glm::min(Position - HalfExtends, other->Position - other->HalfExtends);
	HalfExtends = (max - min) * 0.5f;
	Position = (min + max) * 0.5f;
	SetScale(glm::vec3(1, 1, 1));
}

glm::vec3 CullingAABB::GetPos() const
{
	return Position;
}

glm::vec3 CullingAABB::GetHalfExtends_Unscaled() const
{
	return HalfExtends;
}

glm::vec3 CullingAABB::GetHalfExtends() const
{
	return ScaledHalfExtends;
}

void CullingAABB::Reset()
{
	HalfExtends = glm::vec3();
	Position = glm::vec3();
	ScaledHalfExtends = glm::vec3();
}

void CullingAABB::SetPos(glm::vec3 pos)
{
	Position = pos;
}

void CullingAABB::SetScale(glm::vec3 scale)
{
	Scale = scale;
	ScaledHalfExtends = HalfExtends * Scale;
}


