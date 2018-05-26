
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
	row_major float3x3 TBN:TANGENT1;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

cbuffer GOConstantBuffer : register(b0)
{
	row_major matrix Model;
	int HasNormalMap;
};
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
};

PSInput main(float4 position : POSITION, float4 normal : NORMAL0, float4 uv : TEXCOORD,float4 Tangent: TANGENT0)
{
	PSInput result;
	float4 final_pos = position;
	final_pos.w = 1.0f;
	final_pos = mul(position, Model);
	result.WorldPos = final_pos;
	final_pos = mul(final_pos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos;
	result.uv = uv.xy;
	result.Normal = normal;
	if (HasNormalMap)
	{
		const float3 BiTangent = (mul(float4(cross(normal.xyz, Tangent.xyz).xyz, 0.0), Model));
		const float3 Normal = (mul(float4(normal.xyz, 0.0), Model));
		const float3 tan = (mul(float4(Tangent.xyz, 0.0), Model));
		float3x3 mat = float3x3(tan, BiTangent, Normal);
		result.TBN = mat;
	}
	return result;
}
