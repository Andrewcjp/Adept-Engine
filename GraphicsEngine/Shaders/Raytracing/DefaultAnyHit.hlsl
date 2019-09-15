#include "DefaultShaderCommon.hlsl"

[shader("anyhit")]
void anyhit_main(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
	float3 hitLocation = ObjectRayOrigin() + ObjectRayDirection() * RayTCurrent();
	payload.color += float3(1, 0, 1);

	const float Size = 10;
	float dist = 20;
	if (RayTCurrent() < dist+ Size && RayTCurrent() > dist - Size)
	{
		IgnoreHit();
	}
}