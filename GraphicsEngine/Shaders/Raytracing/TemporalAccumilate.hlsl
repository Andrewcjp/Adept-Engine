
RWTexture2D<float4> RTXUAV : register(u0);
RWTexture2D<float4> LastFrameData : register(u1);
RWTexture2D<uint> SSP_Data: register(u2);
cbuffer ShaderData: register(b0)
{
	int DebugMode;
	int Clear;
}
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupIndex : SV_GroupID)
{	
	if (Clear)
	{
		SSP_Data[DTid.xy] = 0;
		LastFrameData[DTid.xy] = float4(0, 0, 0, 0);
	}
	float4 RTXSample = RTXUAV[DTid.xy];
	float3 NewColour = RTXSample.xyz;
	float4 Oldcolour = LastFrameData[DTid.xy];
	float3 Output = NewColour;
	float3 HistoryOut = Output;
	if (RTXSample.a > 0.995)
	{
		Output = NewColour;
		HistoryOut = float4(0, 0, 0, 0);
		SSP_Data[DTid.xy] = 0;
	}
	else if (RTXSample.a > 0.2f)
	{
		SSP_Data[DTid.xy]++;
		Output = (NewColour+ Oldcolour) / SSP_Data[DTid.xy];
		HistoryOut = Oldcolour+ NewColour;
	}
	else
	{
		SSP_Data[DTid.xy] = 0;
		HistoryOut = float4(0, 0, 0, 0);
		Output = float4(0, 0, 0, 0);
	}
	RTXUAV[DTid.xy] = float4(Output, RTXSample.a);
	LastFrameData[DTid.xy] = float4(HistoryOut, RTXSample.a);
	if (DebugMode == 1)
	{
		float Value = (float)SSP_Data[DTid.xy]/255.0f;
		RTXUAV[DTid.xy] = float4(Value,0,0, RTXSample.a);
	}
	else if (DebugMode == 2)
	{
		RTXUAV[DTid.xy] = RTXSample;
	}
}