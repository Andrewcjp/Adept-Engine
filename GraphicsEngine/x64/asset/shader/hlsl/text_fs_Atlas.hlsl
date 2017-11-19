Texture2D texColour : register(t0);
SamplerState defaultSampler : register (s0);

cbuffer LineConstantBuffer : register(b0)
{
	row_major matrix projection;
	float3 Colour;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	float4 sampled = float4(1.0,1.0,1.0,texColour.Sample(defaultSampler, input.uv).r);
	return float4(Colour,1.0)*sampled;
	//return float4(1,1,1,1);
}


/*#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
	color = vec4(textColor, 1.0) * sampled;

}  */