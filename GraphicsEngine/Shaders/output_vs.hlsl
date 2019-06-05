
cbuffer LineConstantBuffer : register(b0)
{
	row_major matrix projection;
	float3 Colour;
}
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_OUTPUT main(float4 pos : POSITION, float2 uv : TEXCOORD0)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul( float4(pos.x, pos.y, 0.0, 1.0), projection);
	output.uv = pos.zw;
	return output;
}
