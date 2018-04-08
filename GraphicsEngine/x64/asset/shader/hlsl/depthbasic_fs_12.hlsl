//Texture2D texColour : register(t0);
//SamplerState defaultSampler : register (s0);

struct GSOutput
{
	float4 Pos : SV_Position;
	uint slice : SV_RenderTargetArrayIndex;
};

float4 main(GSOutput input) : SV_Target
{
	return float4(1,1,1,1);
}


#if 0
struct FS_Output
{
	float4 out1:SV_Target0;
	float4 out2:SV_Target1;
	float4 out3:SV_Target2;
	float4 out4:SV_Target3;
	float4 out5:SV_Target4;
	float4 out6:SV_Target5;
};
FS_Output main(VS_OUTPUT input) : SV_Target
{

	FS_Output output = (FS_Output)0;
	switch (input.slice)
	{
	case 0:
		output.out1 = float4(1, 1, 1, 1);
		break;
	case 1:
		output.out2 = float4(1, 1, 1, 1);
		break;
	case 2:
		output.out3 = float4(1, 1, 1, 1);
		break;
	case 3:
		output.out4 = float4(1, 1, 1, 1);
		break;
	case 4:
		output.out5 = float4(1, 1, 1, 1);
		break;
	case 5:
		output.out6 = float4(1, 1, 1, 1);
		break;
	}
	
	return output;
}
#endif