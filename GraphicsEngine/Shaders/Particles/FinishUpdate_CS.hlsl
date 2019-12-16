#include "Particle_Common.hlsl"
StructuredBuffer<uint> AliveIndexs : register(t10); // UAV
RWByteAddressBuffer IndirectCommandBuffer : register(u1);
RWByteAddressBuffer Counter : register(u2);
[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	//this returns 0 on a 2070?
    uint PostsimCount = Counter.Load(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION);
   // if (DTid.x < PostsimCount)
    {
        const int Stride = 20; //size of the indirect args
        const int index = DTid.x;
        IndirectCommandBuffer.Store(Stride * index, AliveIndexs[index]);
    }
}


