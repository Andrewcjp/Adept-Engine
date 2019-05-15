
float3 GetReflectionColor(float3 R,float Roughness)
{
	//todo: Find relevant probe
	//blend them
	return SpecularBlurMap[0].SampleLevel(defaultSampler, R, Roughness * (MAX_REFLECTION_LOD)).rgb;
}