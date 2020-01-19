Texture2D<float4> VxIn: register(t0);
RWTexture2D<float4> RTXUAV : register(u0);
cbuffer RTBufferData: register(b0)
{
	float RT_RoughnessThreshold;
	float VX_MaxRoughness;
	float VX_MinRoughness;
	float VX_RT_BlendStart;
	float VX_RT_BlendEnd;
	float VX_RT_BlendFactor;
};

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupIndex : SV_GroupID)
{
	float4 Vxcolour = VxIn[DTid.xy];
	float4 RTcolour = RTXUAV[DTid.xy];
	float OutA = max(Vxcolour.a, RTcolour.a);

	float3 FinalColour = Vxcolour.xyz;
	if (OutA > VX_RT_BlendStart && OutA < VX_RT_BlendEnd)
	{
		FinalColour = (RTcolour.xyz+ Vxcolour.xyz)/2;
	}
	else if (OutA > RT_RoughnessThreshold)
	{
		FinalColour = RTcolour.xyz;
	}
	else 
	{
		FinalColour = Vxcolour.xyz;
	}	
	RTXUAV[DTid.xy] = float4(FinalColour, OutA);
}