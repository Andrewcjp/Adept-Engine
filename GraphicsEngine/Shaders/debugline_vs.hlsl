cbuffer ConstantBuffer : register(b0)
{
	row_major matrix projection;
}

struct VS_OUTPUT
{
	float4 pos    : SV_POSITION;
	float3 colour : TEXCOORD0;
};


VS_OUTPUT main(float3 pos : POSITION, float3 color : NORMAL0)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
#if TWOD_ONLY
	pos.z = 0.0f;
#endif
	output.pos = mul(float4(pos, 1.0), projection);
	output.colour = color;
	return output;
}
