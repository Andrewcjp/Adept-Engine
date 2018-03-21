cbuffer ConstantBuffer : register(b0)
{
	row_major matrix projection;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 FrontColour:TEXCOORD;
	float3 BackColour:TEXCOORD1;
	uint Back: TEXCOORD2;
};


VS_OUTPUT main(float2 position : POSITION, uint back : TEXCOORD2, float3 fc : TEXCOORD, float3 bc : TEXCOORD1)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 finalpos = float4(position.xy, 0.0, 1.0);
	output.pos = mul(finalpos, projection);
	output.FrontColour = fc;
	output.Back = back;
	output.BackColour = bc;	
	return output;
}
