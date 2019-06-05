
struct PrimitiveData
{
	row_major matrix Model;
#if !VULKAN
	float4x4 pad;
	float4x4 pad2;
	float4x4 pad3;
#endif
};

#if USE_INSTANCING
PrimitiveData GetPrimitiveData(uint PrimitiveId)
{
	return InstancedData[PrimitiveId];
}
#else
#define GetPrimitiveData(x) Primitive 
#endif

inline uint flatten2D(uint2 coord, uint2 dim)
{
	return coord.x + coord.y * dim.x;
}