
#include "DebugDrawers.h"
#include "Rendering/Core/DebugLineDrawer.h"
#define PI (3.1415926535897932f)
static const float DefultLifeTime = 50.0f;
void DebugDrawers::DrawDebugSphere(glm::vec3 const & Center, float Radius, glm::vec3 const & Color, int Segments, bool bPersistentLines, float LifeTime, float Thickness)
{
	float LineLifeTime = (LifeTime > 0.f) ? LifeTime : DefultLifeTime;

	// Need at least 4 segments
	Segments = std::max(Segments, 4);

	glm::vec3 Vertex1, Vertex2, Vertex3, Vertex4;
	const float AngleInc = 2.f * PI / float(Segments);
	int NumSegmentsY = Segments;
	float Latitude = AngleInc;
	int NumSegmentsX;
	float Longitude;
	float SinY1 = 0.0f, CosY1 = 1.0f, SinY2, CosY2;
	float SinX, CosX;
	while (NumSegmentsY--)
	{
		SinY2 = glm::sin(Latitude);
		CosY2 = glm::cos(Latitude);

		Vertex1 = glm::vec3(SinY1, 0.0f, CosY1) * Radius + Center;
		Vertex3 = glm::vec3(SinY2, 0.0f, CosY2) * Radius + Center;
		Longitude = AngleInc;

		NumSegmentsX = Segments;
		while (NumSegmentsX--)
		{
			SinX = glm::sin(Longitude);
			CosX = glm::cos(Longitude);

			Vertex2 = glm::vec3((CosX * SinY1), (SinX * SinY1), CosY1) * Radius + Center;
			Vertex4 = glm::vec3((CosX * SinY2), (SinX * SinY2), CosY2) * Radius + Center;

			DrawDebugLine(Vertex1, Vertex2, Color, false, LineLifeTime, Thickness);
			DrawDebugLine(Vertex1, Vertex3, Color, false, LineLifeTime, Thickness);

			Vertex1 = Vertex2;
			Vertex3 = Vertex4;
			Longitude += AngleInc;
		}
		SinY1 = SinY2;
		CosY1 = CosY2;
		Latitude += AngleInc;
	}
}

void DebugDrawers::DrawCircle(const glm::vec3& Base, const glm::vec3& X, const glm::vec3& Y, const glm::vec3& Color, float Radius, int NumSides, bool bPersistentLines, float LifeTime, float Thickness)
{
	const float	AngleDelta = 2.0f * PI / NumSides;
	glm::vec3	LastVertex = Base + X * Radius;

	for (int SideIndex = 0; SideIndex < NumSides; SideIndex++)
	{
		const glm::vec3 Vertex = Base + (X * glm::cos(AngleDelta * (SideIndex + 1)) + Y * glm::sin(AngleDelta * (SideIndex + 1))) * Radius;
		DrawDebugLine(LastVertex, Vertex, Color, bPersistentLines, LifeTime, Thickness);
		LastVertex = Vertex;
	}
}

void DebugDrawers::DrawHalfCircle(const glm::vec3& Base, const glm::vec3& X, const glm::vec3& Y, const glm::vec3& Color, float Radius, int NumSides, bool bPersistentLines, float LifeTime, float Thickness)
{
	const float	AngleDelta = 2.0f * PI / (float)NumSides;
	glm::vec3	LastVertex = Base + X * Radius;

	for (int SideIndex = 0; SideIndex < (NumSides / 2); SideIndex++)
	{
		const glm::vec3 Vertex = Base + (X * glm::cos(AngleDelta * (SideIndex + 1)) + Y * glm::sin(AngleDelta * (SideIndex + 1))) * Radius;
		DrawDebugLine(LastVertex, Vertex, Color, bPersistentLines, LifeTime, Thickness);
		LastVertex = Vertex;
	}
}

void DebugDrawers::DrawDebugLine(glm::vec3 const& LineStart, glm::vec3  const& LineEnd, glm::vec3  const& Color, bool bPersistentLines, float LifeTime, float Thickness)
{
	if (DebugLineDrawer::Get() == nullptr)
	{
		AssertDebugBreak();
		return;
	}
	//todo: persistent lines
	//todo: thickness
	DebugLineDrawer::Get()->AddLine(LineStart, LineEnd, Color, LifeTime);
}

void DebugDrawers::DrawDebugCapsule(glm::vec3 const& Center, float HalfHeight, float Radius, glm::quat& Rotation, glm::vec3 const& Color, bool bPersistentLines, float LifeTime, float Thickness)
{
	const int DrawCollisionSides = 16;

	glm::vec3 Origin = Center;
	glm::vec3 XAxis = glm::vec4(1, 0, 0, 0) * glm::mat4(Rotation);
	glm::vec3 YAxis = glm::vec4(0, 1, 0, 0) * glm::mat4(Rotation);
	glm::vec3 ZAxis = glm::vec4(0, 0, 1, 0) * glm::mat4(Rotation);

	// Draw top and bottom circles
	float HalfAxis = std::max<float>(HalfHeight - Radius, 1.f);
	glm::vec3 TopEnd = Origin + HalfAxis * ZAxis;
	glm::vec3 BottomEnd = Origin - HalfAxis * ZAxis;

	DrawCircle(TopEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, Thickness);
	DrawCircle(BottomEnd, XAxis, YAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, Thickness);

	// Draw domed caps
	DrawHalfCircle(TopEnd, YAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, Thickness);
	DrawHalfCircle(TopEnd, XAxis, ZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, Thickness);

	glm::vec3 NegZAxis = -ZAxis;

	DrawHalfCircle(BottomEnd, YAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, Thickness);
	DrawHalfCircle(BottomEnd, XAxis, NegZAxis, Color, Radius, DrawCollisionSides, bPersistentLines, LifeTime, Thickness);

	// Draw connected lines
	DrawDebugLine(TopEnd + Radius * XAxis, BottomEnd + Radius * XAxis, Color, bPersistentLines, LifeTime, Thickness);
	DrawDebugLine(TopEnd - Radius * XAxis, BottomEnd - Radius * XAxis, Color, bPersistentLines, LifeTime, Thickness);
	DrawDebugLine(TopEnd + Radius * YAxis, BottomEnd + Radius * YAxis, Color, bPersistentLines, LifeTime, Thickness);
	DrawDebugLine(TopEnd - Radius * YAxis, BottomEnd - Radius * YAxis, Color, bPersistentLines, LifeTime, Thickness);
}

void DebugDrawers::FlushAllLines()
{
	DebugLineDrawer::Get()->FlushDebugLines();
}
