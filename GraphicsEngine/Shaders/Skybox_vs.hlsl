
struct PSInput
{
	float4 position : SV_POSITION;
	float3 texcoord :TEXCOORD;
};

cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 View;
	float4x4 Projection;
};

PSInput main(float4 position : POSITION)
{
	PSInput result;
	float4 final_pos = float4(position.xyz, 0.0);
	final_pos = mul(final_pos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos.xyww;
	result.texcoord = normalize(position.xyz);
	return result;
}
