struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_OUTPUT main(float4 position : POSITION)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = position;
	const float2 fliped = float2 (position.x, -position.y);
	output.uv = (fliped + float2(1, 1)) / 2.0f;
	return output;
}
