
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 FrontColour:TEXCOORD;
	float3 BackColour:TEXCOORD1;
	int Back : TEXCOORD2;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	if (input.Back == 0 )
	{
		return float4(input.FrontColour.xyz, 0.0);
	}
	else
	{
		return float4(input.BackColour.xyz,0.0);
	}
	
}
