RWBuffer<uint>	 Counter: register(u0);	// UAV
RWByteAddressBuffer  IndirectCommandBuffer: register(u1);
[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint count = Counter.Load(0);
	if (count < 100)
	{
		IndirectCommandBuffer.Store(0, count);
		IndirectCommandBuffer.Store(12, count);
	}	
}


