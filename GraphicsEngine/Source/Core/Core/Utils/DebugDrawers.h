#pragma once
class DebugDrawers
{
public:
	struct FrustumData
	{
		float FrustumAngle;
		float FrustumStartDist;
		float FrustumAspectRatio;
		float FrustumEndDist;
		glm::mat4 Transform;
	};
	CORE_API static void DrawDebugLine(glm::vec3 const & LineStart, glm::vec3 const & LineEnd, glm::vec3 const & Color = glm::vec3(1), bool bPersistentLines = false, float LifeTime = 0.0f, float Thickness = 1.0f);
	CORE_API static void DrawDebugSphere(glm::vec3 const& Center, float Radius, glm::vec3 const& Color, int Segments = 12, bool bPersistentLines = false, float LifeTime = 0.0f, float Thickness = 1.0f);
	CORE_API static void DrawCircle(const glm::vec3 & Base, const glm::vec3 & X, const glm::vec3 & Y,
		const glm::vec3 & Color = glm::vec3(1), float Radius = 1, int NumSides = 16, bool bPersistentLines = false, float LifeTime = 0.0f, float Thickness = 1.0f);
	CORE_API static void DrawHalfCircle(const glm::vec3 & Base, const glm::vec3 & X, const glm::vec3 & Y,
		const glm::vec3 & Color = glm::vec3(1), float Radius = 1, int NumSides = 16, bool bPersistentLines = false, float LifeTime = 0.0f, float Thickness = 1.0f);

	CORE_API static void DrawDebugCapsule(glm::vec3 const & Center, float HalfHeight, float Radius, glm::quat & Rotation,
		glm::vec3 const & Color = glm::vec3(1), bool bPersistentLines = false, float LifeTime = 0.00f, float Thickness = 1.0f);
	CORE_API static void FlushAllLines();
	CORE_API static void DrawDebugBox(glm::vec3 const & Position, glm::vec3 const & HalfExtends, const glm::quat & Rotation, glm::vec3 const & colour, bool bPersistentLines, float time);

	static DebugDrawers::FrustumData CreateFromCam(Camera * cam);
	static void DrawCameraFrustum(Camera* cam);
	static void DrawCameraFrustum(FrustumData cam);
private:

};
