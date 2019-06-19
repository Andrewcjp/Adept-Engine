StructuredBuffer<uint> LightList : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
RWTexture2D<float4> SrcTex : register(u1);
SamplerState BilinearClamp : register(s0);
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	uint2 TileCount;
	//	Light lights[MAX_LIGHTS];
};
inline uint flatten2D(uint2 coord, uint2 dim)
{
	return coord.x + coord.y * dim.x;
}
[numthreads(LIGHTCULLING_TILE_SIZE, LIGHTCULLING_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint  groupIndex : SV_GroupIndex)
{
	int index = flatten2D(groupID.xy, TileCount)*MAX_LIGHTS;
	uint lightcount = LightList[index];
	float Scale = (float)lightcount / (float)MAX_LIGHTS;
	float3 heat = lerp(float3(1, 0, 0), float3(0, 0, 1), 1.0 - Scale);
	if (lightcount > MAX_LIGHTS)
	{
		heat = float3(1, 1, 1);
	}
	if (lightcount == 0)
	{
		heat = float3(0, 0, 0);
	}
	DstTexture[DTid.xy] = SrcTex[DTid.xy] * 0.4 + float4(heat, 1.0f);
}
