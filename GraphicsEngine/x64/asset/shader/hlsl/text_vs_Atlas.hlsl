
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


VS_OUTPUT main(float4 pos : POSITION)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = mul( float4(pos.x, pos.y, 0.0, 1.0), projection);
	output.uv = pos.zw;
	return output;
}



/*
#version 330 core
layout(location = 0) in vec4 vertex; // vec2 pos & vec2 texure coods
out vec2 TexCoords;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
} 
*/