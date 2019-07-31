Texture2DArray<float4> SRCLevel1 : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);

[numthreads(4, 4, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    const float factor = 4;
    const float height = 681*2;//340*factor;
    const float width = 1684*2;//842*factor;

    float3 pos = float3(DTid.x/width,DTid.y/height,1);

	float4 Inputcolour=  SRCLevel1.SampleLevel(BilinearClamp,pos,0);
    if( Inputcolour.a > 0.5)
    {
	    DstTexture[DTid.xy] = Inputcolour;
    }
}



