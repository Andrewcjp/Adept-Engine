
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
