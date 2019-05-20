cbuffer SceneConstantBuffer : register(b0)
{
	row_major matrix View;
	row_major matrix Projection;
};

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
	output.pos = mul(float4(pos, 1.0), View);
#else
	pos = mul(float4(pos, 1.0), View).xyz;	
	output.pos = mul(float4(pos, 1.0), Projection);
#endif	
	output.colour = color;
	return output;
}
