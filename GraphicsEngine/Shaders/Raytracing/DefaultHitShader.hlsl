#include "DefaultShaderCommon.hlsl"

[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
	uint instanceID = InstanceID();
	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

	const float3 A = float3(1, 0, 0);
	const float3 B = float3(0, 1, 0);
	const float3 C = float3(0, 0, 1);
	payload.color = C * barycentrics.x + A * barycentrics.y + B * barycentrics.z;

	switch (instanceID)
	{
		case 0:
			payload.color = A * barycentrics.x + B * barycentrics.y + C * barycentrics.z;
			break;
		case 1:
			payload.color = B * barycentrics.x + C * barycentrics.y + A * barycentrics.z;
			break;
		case 2:
			payload.color = C * barycentrics.x + A * barycentrics.y + B * barycentrics.z;
			break;
	}
	//payload.color = float3(((float)instanceID) / 30, 0, 0);
}
