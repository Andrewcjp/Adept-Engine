#include "Particle_Common.hlsl"
RWStructuredBuffer<PosVelo> newPosVelo : register(u0); // UAV
RWByteAddressBuffer CounterBuffer : register(u1);
RWStructuredBuffer<uint> AliveIndexs : register(u2);
RWStructuredBuffer<uint> DeadIndexs : register(u3);

[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint EmitCount = CounterBuffer.Load(PARTICLECOUNTER_OFFSET_EMITCOUNT);
	if (DTid.x < EmitCount)
	{
		uint newParticleIndex = 0;
		uint DeadCount = CounterBuffer.Load(PARTICLECOUNTER_OFFSET_DEADCOUNT);
		if (DeadCount > 1)
		{
			CounterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, (uint)-1, DeadCount);
			newParticleIndex = DeadIndexs[DeadCount - 1];

			uint aliveCount;
			CounterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_ALIVECOUNT, (uint)1, aliveCount);
			AliveIndexs[aliveCount] = newParticleIndex;

			newPosVelo[newParticleIndex] = (PosVelo)0;
			newPosVelo[newParticleIndex].pos = float4(0, 20, 0, 2);
			int Tweak = newParticleIndex % 10;
			newPosVelo[newParticleIndex].vel = float4(Tweak, 5, 5, 0);
			newPosVelo[newParticleIndex].Lifetime = 20;
			newPosVelo[newParticleIndex].Size = 0.1f;
		}
	}
}


