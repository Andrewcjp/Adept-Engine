
struct PSInput
{
	float4 position : SV_POSITION;
	float4 LastPos :TEXCOORD;
	float4 CurrentPos :TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
	float2	lastPos = (input.LastPos.xy / input.LastPos.w);
	float2 CurrentPos = (input.CurrentPos.xy / input.CurrentPos.w);
	float2 output = CurrentPos.xy - lastPos.xy;
	return float4(output,0,0);//output in NDC space
}
