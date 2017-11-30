
cbuffer LightBuffer : register(b1)
{
	float3 LPosition;	
	float3 color;
	float3 Direction;
};
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);

Texture2D g_Shadow_texture : register(t4);
SamplerState g_sampler : register(s0);
float GetShadow(float4 pos)
{
	//float4 vLightSpacePos = vPosWorld;
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].view);
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].projection);

	//vLightSpacePos.xyz /= vLightSpacePos.w;

	float3 fragtolight = Direction;
	if (g_Shadow_texture.Sample(g_sampler, fragtolight.xy).r < pos.z )
	{
		return 1.0f;
	}
	return 0.0f;
}

float4 main(PSInput input) : SV_TARGET
{
	float4 output = g_texture.Sample(g_sampler, input.uv);
	 output *= float4(color.x, color.y, color.z,1.0);
	 //return float4(GetShadow(input.position), GetShadow(input.position), GetShadow(input.position), 1.0);
	 return output;// *GetShadow(input.position);
}
