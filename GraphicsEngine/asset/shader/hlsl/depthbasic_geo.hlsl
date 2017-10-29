Texture2D texColour : register( t0 );
SamplerState defaultSampler : register ( s0 );

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 normal : NORMAL0;
	float3 uv : TEXCOORD0;
};

float4 main( VS_OUTPUT input ) : SV_Target
{
    return texColour.Sample( defaultSampler, input.uv );
}
