#include "DefaultShaderCommon.hlsl"
[shader("intersection")]
void intersection_main()
{
	BuiltInTriangleIntersectionAttributes ar;
	ar.barycentrics = float2(1, 1);
	ReportHit(RayTCurrent()+1, 0, ar);
}