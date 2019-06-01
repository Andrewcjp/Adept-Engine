sampler2D TExture : register(t1)
cbuffer ConstantBuffer : register(b0)
{
	float4 color;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	return float4(color.xyz,1.0f);
	return float4(1,color.y,0,1);
}