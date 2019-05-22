StructuredBuffer<uint> LightList : register(t1);
struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD0;
	float4 WorldPos:TANGENT0;
	row_major float3x3 TBN:TANGENT1;
};

//Declares
float4 main(PSInput input) : SV_TARGET
{
	int x = LightList[16];
	//return float4(x, 0, 0, 1.0f);
	return float4(input.position.xyz, 1.0f);
}
