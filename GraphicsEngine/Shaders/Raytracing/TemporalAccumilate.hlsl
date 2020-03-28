
RWTexture2D<float4> RTXUAV : register(u0);
RWTexture2D<float4> LastFrameData : register(u1);
RWTexture2D<uint> SSP_Data: register(u2);
Texture2D Velocity: register(t0);
cbuffer ShaderData: register(b0)
{
	int DebugMode;
	int Clear;
}
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupIndex : SV_GroupID)
{	
	uint2 HistoryPos = DTid.xy;
#ifdef WITH_VEL
	float2 PixelVelocity = Velocity[DTid.xy] * float2(1920, 1080);
	//if (length(PixelVelocity) > 0.01f)
	{
		HistoryPos -= PixelVelocity;
	}
#else
	float2 PixelVelocity = Velocity[DTid.xy] * float2(1920, 1080);
	if (length(PixelVelocity) > 0.01f)
	{
		HistoryPos = DTid.xy;
		SSP_Data[DTid.xy] = 0;
		LastFrameData[DTid.xy] = float4(0, 0, 0, 0);
	}
#endif
	if (Clear)
	{
		HistoryPos = DTid.xy;
		SSP_Data[DTid.xy] = 0;
		LastFrameData[DTid.xy] = float4(0, 0, 0, 0);
	}
	float4 RTXSample = RTXUAV[DTid.xy];
	float3 NewColour = RTXSample.xyz;
	float4 Oldcolour = LastFrameData[HistoryPos.xy];
	int SSP = SSP_Data[HistoryPos.xy];
	float3 Output = NewColour;
	float3 HistoryOut = Output;
	if (RTXSample.a > 0.995)
	{
		Output = NewColour;
		HistoryOut = float4(0, 0, 0, 0);
		SSP = 0;
	}
	else if (RTXSample.a > 0.2f)
	{
		SSP++;
		Output = (NewColour + Oldcolour) / SSP;
		HistoryOut = Oldcolour+ NewColour;
	}
	else
	{
		SSP = 0;
		HistoryOut = float4(0, 0, 0, 0);
		Output = float4(0, 0, 0, 0);
	}
	SSP_Data[DTid.xy] = SSP;
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