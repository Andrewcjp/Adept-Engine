float2 D3DX_R16G16_FLOAT_to_FLOAT2(uint packedInput)
{
	float2 unpackedOutput;
	unpackedOutput.x = f16tof32(packedInput & 0x0000ffff);
	unpackedOutput.y = f16tof32(packedInput >> 16);
	return unpackedOutput;
}

uint D3DX_FLOAT2_to_R16G16_FLOAT(float2 unpackedInput)
{
	uint packedOutput;
	packedOutput = asuint(f32tof16(unpackedInput.x)) |
		(asuint(f32tof16(unpackedInput.y)) << 16);
	return packedOutput;
}