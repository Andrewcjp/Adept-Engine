cbuffer ConstantBuffer : register(b0)
{
	row_major matrix world;
	row_major matrix view;
	row_major matrix projection;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
//	float3 normal:NORMAL0;
	float2 uv : TEXCOORD0;
};


VS_OUTPUT main(float4 position : POSITION, /*float4 normal : NORMAL0,*/ float4 uv : TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 final_pos = mul(position, world);

	//final_pos = mul(final_pos, view);
	//final_pos = mul(final_pos, projection);
//	output.normal = float3(0, 0, -1);
	output.pos = float4(position.xy, 0, 10.0f);

	//output.uv = uv;
	float2 u = (position.xy + float2(1, 1)) / 2.2;
	output.uv = uv;// u;///float3(u.x, -u.y, 0);
	return output;
}
