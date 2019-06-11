#include "DefaultShaderCommon.hlsl"

[shader("miss")]
void miss(inout RayPayload payload)
{
	payload.color = float3(0.4, 0.6, 0.2);
}