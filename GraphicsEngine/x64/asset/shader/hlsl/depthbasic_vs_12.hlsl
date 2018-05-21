
cbuffer ConstantBuffer : register(b0)
{
	row_major matrix world;
}
#if 0
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix ViewP;
	row_major matrix Projection;
};

#else
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix ViewP;
	row_major matrix Projection;
	float4 LightPos;
};
#endif
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float3 LightPos: TEXCOORD0;
	float3 WorldPos:NORMAL0;
};


VS_OUTPUT main(float4 pos : POSITION, float4 normal : NORMAL0, float3 uv : TEXCOORD0)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 final_pos = mul(float4(pos.xyz,1.0), world);
#if DIRECTIONAL
	final_pos = mul(final_pos, ViewP);
	final_pos = mul(final_pos, Projection);
#endif
	output.pos = final_pos;
	output.LightPos = LightPos;
	return output;
}
