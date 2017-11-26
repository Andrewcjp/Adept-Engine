//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
	float4 offset;
	row_major matrix Model;
	row_major matrix View;
	row_major matrix Projection;
	
};


PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL0, float4 uv : TEXCOORD)
{
	PSInput result;
	float4 final_pos = position;
	final_pos.w = 1.0f;
	final_pos = mul(position, Model);
	final_pos = mul(final_pos, View);
	final_pos = mul(final_pos, Projection);
	result.position = final_pos;
	result.uv = uv.xy;
	result.Normal = normal;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return g_texture.Sample(g_sampler, input.uv);
}
