
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD0;
	float4 WorldPos:TANGENT0;
	row_major float3x3 TBN:TANGENT1;
};

float4 main(PSInput input) : SV_TARGET
{
	return float4(1.0,1.0,1.0,1.0f);
}
