Texture2D<float4> SrcTexture : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float4 Inputcolour = SrcTexture[DTid.xy];
	//vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
	//float result = 0.0;
	//for (int x = -2; x < 2; ++x)
	//{
	//	for (int y = -2; y < 2; ++y)
	//	{
	//		vec2 offset = vec2(float(x), float(y)) * texelSize;
	//		result += texture(ssaoInput, TexCoords + offset).r;
	//	}
	//}
	//FragColor = result / (4.0 * 4.0);
}
