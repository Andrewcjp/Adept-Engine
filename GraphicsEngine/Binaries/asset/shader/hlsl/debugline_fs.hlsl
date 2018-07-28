
struct VS_OUTPUT
{
	float4 pos    : SV_POSITION;
	float3 colour : TEXCOORD0;
};
float4 main(VS_OUTPUT input) : SV_Target
{
	return float4(input.colour,1.0f);
}
