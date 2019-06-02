
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal: SV_NORMAL;
	float2 uv : TEXCOORD;
};

cbuffer GOConstantBuffer : register(b0)
{
	row_major matrix Model;
};
cbuffer SceneConstantBuffer : register(b3)
{
	row_major matrix View;
	row_major matrix Projection;
};

PSInput main(float4 position : POSITION, float4 normal : NORMAL0, float4 uv : TEXCOORD)
{
	PSInput result = (PSInput)0;
	float4 final_pos = position;
	final_pos =  mul(float4(position.xyz,1.0f), Model);
	//result.WorldPos = final_pos;

	final_pos = mul(final_pos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos;

	result.uv = uv.xy;
	result.Normal = normal;// mul(normal, SumTrans);
	result.position.y = -result.position.y;
	return result;
}
