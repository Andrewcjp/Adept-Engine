struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD0;
	uint slice : SV_RenderTargetArrayIndex;
};


cbuffer constantBuffer : register(b0)
{
	row_major matrix ViewProj;
	int slice : SV_RenderTargetArrayIndex;	
}



VS_OUTPUT main(float4 position : POSITION )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 final_pos = float4(position.xyz, 1.0);
	final_pos = mul(final_pos, ViewProj);
	output.pos = final_pos.xyww;
	output.slice = slice;
	output.uv = normalize(position.xyz);
	return output;
}
