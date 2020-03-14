#if !FULL_UAV_LOAD
RWTexture2D<float4> FinalOut: register(u1);
RWTexture2DArray<uint> PackedInput : register(u0);
#include "Utils/UavLoad.hlsl"

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupIndex : SV_GroupID)
{
	FinalOut[DTid.xy] = LoadUAV(DTid.xy, PackedInput);
}
#endif