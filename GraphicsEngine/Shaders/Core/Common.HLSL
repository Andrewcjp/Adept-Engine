
struct PrimitiveData
{
	row_major matrix world;
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