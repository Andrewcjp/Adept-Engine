
struct PrimitiveData
{
	row_major matrix Model;
	
#if !VULKAN
	float4x4 Data;
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
#if WITH_INSTANCING
#define GetMatData(x) Parms[input.id].x
#define INSTANCINGPSDATA uint id : SV_InstanceID;
#else
#define GetMatData(x) Parms[0].x
#define INSTANCINGPSDATA
#endif

inline uint flatten2D(uint2 coord, uint2 dim)
{
	return coord.x + (coord.y * dim.x);
}

#if VULKAN
#define PUSHCONST [[vk::push_constant]] 
#else
#define PUSHCONST
#endif