TextureCube texColour : register(t0);
SamplerState defaultSampler : register (s0);

cbuffer Data : register(b0)
{
	float3 Normal: Normal0;
	uint slice : SV_RenderTargetArrayIndex;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD0;
	uint slice : SV_RenderTargetArrayIndex;
};

static const float PI = 3.14159265359;

float4 main(VS_OUTPUT input) : SV_Target
{
	//return float4(Normal,1.0f);
	//return float4(slice,slice,slice,1.0);
	float3 irradiance = float3(0.0,0,0);
	float3 target = float3(input.uv.x,input.uv.y, input.uv.z);
	//return float4(1, 1, 1, 1);
	//return float4(texColour.Sample(defaultSampler, target).rgb, 1.0);

	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = cross(up, target);
	up = cross(target, right);

	float sampleDelta = 0.25;
	int nrSamples = 0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * target;

			irradiance += texColour.SampleLevel(defaultSampler, sampleVec,0).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0f);

	//
}
