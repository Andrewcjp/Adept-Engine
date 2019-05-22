StructuredBuffer<uint> LightList : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
RWTexture2D<float4> SrcTex : register(u1);
SamplerState BilinearClamp : register(s0);
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	int2 TileCount;
	//	Light lights[MAX_LIGHTS];
};
inline uint flatten2D(uint2 coord, uint2 dim)
{
	return coord.x + coord.y * dim.x;
}
[numthreads(LIGHTCULLING_TILE_SIZE, LIGHTCULLING_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint  groupIndex : SV_GroupIndex)
{
	//	DstTexture[DTid.xy] = float4(0, 0, 0, 1);
		//int2 tilecount = int2(480, 270);

	int index = flatten2D(groupID.xy, TileCount)*MAX_LIGHTS;
	int lightcount = LightList[index];
	float4 heat = float4((float)lightcount / (float)MAX_LIGHTS, 0, 0, 1);
	if (lightcount > MAX_LIGHTS)
	{
		heat = float4(1, 1, 1, 1);
	}
	DstTexture[DTid.xy] = SrcTex[DTid.xy] * 0.4 + heat;
}
