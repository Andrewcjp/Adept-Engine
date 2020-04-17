Texture3D<float4> SrcTexture : register(t0);
RWTexture3D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);

PUSHCONST cbuffer CB : register(b0)
{
	float3 TexelSize;   // 1.0 / destination dimension
}

[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	//DTid is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
	//As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
	//TexelSize = 1.0 / 256;
	//(1.0 / 256) 
	float3 texcoords = TexelSize * (DTid.xyz + 0.5);

	//The samplers linear interpolation will mix the four pixel values to the new pixels color
	float4 color = SrcTexture.SampleLevel(BilinearClamp, texcoords, 0);

	//Write the final color into the destination texture.
	DstTexture[DTid.xyz] = color;
}