
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float4 WorldPos:TANGENT0;
};

cbuffer GOConstantBuffer : register(b0)
{
	row_major matrix Model;
	int HasNormalMap;
	float Roughness;
	float Metallic;
};
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
};

PSInput main(float4 position : POSITION, float4 normal : NORMAL0, float4 uv : TEXCOORD,float4 Tangent: TANGENT0)
{
	PSInput result =(PSInput)0;
	float4 final_pos = position;
	final_pos.w = 1.0f;
	final_pos = mul(position, Model);
	result.WorldPos = final_pos;
	final_pos = mul(final_pos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos;
	result.Normal = normal;
	return result;
}
