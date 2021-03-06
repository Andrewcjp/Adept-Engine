cbuffer ConstantBuffer : register(b0)
{
	row_major matrix world;
	row_major matrix view;
	row_major matrix projection;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD0;
};


VS_OUTPUT main(float4 pos : POSITION, float3 uv : TEXCOORD0)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 final_pos = mul(pos, world);

	//final_pos = mul(final_pos, view);
	//final_pos = mul(final_pos, projection);

	output.pos = final_pos;

	//output.uv = uv;
	float2 u = (pos.xy + float2(1, 1)) / 2.0;
	output.uv = float3(u.x, -u.y, 0);
	return output;
}
