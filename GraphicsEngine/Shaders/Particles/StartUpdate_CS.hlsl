#include "Particle_Common.hlsl"
RWByteAddressBuffer	 Counter: register(u0);	// UAV
RWByteAddressBuffer  IndirectCommandBuffer: register(u1);
cbuffer emitData: register(b0)
{
	int EmitCount;
};
[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    uint count = Counter.Load(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION);
    uint deadcount = Counter.Load(PARTICLECOUNTER_OFFSET_DEADCOUNT);
    uint Ecount = min(EmitCount, deadcount);
    Counter.Store(PARTICLECOUNTER_OFFSET_EMITCOUNT, Ecount);
    IndirectCommandBuffer.Store(0, Ecount);
    IndirectCommandBuffer.Store(12, count + Ecount);
	//reset the particle count		
    uint LastFrameAliveCount = Counter.Load(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION);
    Counter.Store(PARTICLECOUNTER_OFFSET_ALIVECOUNT, LastFrameAliveCount);
    Counter.Store(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION, 0);
}


