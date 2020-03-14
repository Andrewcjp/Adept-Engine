#define VRS_TILE_SIZE 16
struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
Texture2D<float> EdgeTex;
float4 main(PSInput input) : SV_TARGET
{
	uint2 Resolution = uint2(1386,681);
	uint2 Origin = input.uv*Resolution;
	int EdgesHit = 0;
	for (int x = 0; x < VRS_TILE_SIZE; x += 1)
	{
		for (int y = 0; y < VRS_TILE_SIZE; y += 1)
		{
			int2 samplepos = Origin + int2(x, y);
			if (samplepos.x >= Resolution.x || samplepos.y >= Resolution.y)
			{
				continue;
			}
			if (EdgeTex[samplepos].r > 0.2)
			{
				EdgesHit++;
			}
		}
	}	
	float Scale = (float)EdgesHit / (float)255;
	float3 heat = lerp(float3(1, 0, 0), float3(0, 0, 1), 1.0 - saturate(Scale));
	return float4(heat,1);
}


