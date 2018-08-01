
struct PSInput
{
	float4 position : SV_POSITION;
	float3 texcoord :TEXCOORD;
};

cbuffer SceneConstantBuffer : register(b0)
{
	row_major matrix View;
	row_major matrix Projection;
};

PSInput main(float4 position : POSITION)
{
	PSInput result;
	float4 final_pos = float4(position.xyz,1.0);
	final_pos = mul(final_pos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos.xyww;
	result.texcoord = normalize(position.xyz);
	return result;
}
