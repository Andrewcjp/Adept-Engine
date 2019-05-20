cbuffer ConstantBuffer : register( b0 )
{
	row_major matrix world;
	row_major matrix view;
	row_major matrix projection;
}
cbuffer GOConstantBuffer : register(b0)
{
	row_major matrix Model;
	int HasNormalMap;
	float Roughness;
	float Metallic;
};
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float3 uv : TEXCOORD0;
};


VS_OUTPUT main( float4 pos : POSITION, float4 normal : NORMAL0, float3 uv : TEXCOORD0 )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 final_pos = mul( pos, world );
	result.WorldPos = final_pos;
	final_pos = mul( final_pos, view );
	final_pos = mul( final_pos, projection );

	output.pos = final_pos;

	output.normal = mul(normal, world);
	output.uv = uv;

    return output;
}
