Texture2D<float4> SrcTexture : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float4 Inputcolour = SrcTexture[DTid.xy];

	//float exposure = 2.5f;
	//Inputcolour = float4(1.0, 1.0, 1.0, 1.0) - exp(-Inputcolour * exposure);
	float Brightness = dot(Inputcolour.rgb, float3(0.2126, 0.7152, 0.0722));
	if (Brightness >= 0.9f)
	{
		DstTexture[DTid.xy] = Inputcolour;
	}
	else 
	{
		DstTexture[DTid.xy] = float4(0,0,0,0.0f);
	}
}
