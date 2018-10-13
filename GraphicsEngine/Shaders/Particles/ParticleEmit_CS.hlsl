#include "Particle_Common.hlsl"
RWStructuredBuffer<PosVelo> newPosVelo	: register(u0);	// UAV
RWByteAddressBuffer CounterBuffer: register(u1);
RWStructuredBuffer<uint> AliveIndexs :register(u2);
RWStructuredBuffer<uint> DeadIndexs :register(u3);

[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint EmitCount = CounterBuffer.Load(PARTICLECOUNTER_OFFSET_EMITCOUNT);
	if (DTid.x < EmitCount)
	{
		uint newParticleIndex = 0;

		uint DeadCount;
		CounterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, -1, DeadCount);
		if (DeadCount > 1)
		{
			newParticleIndex = DeadIndexs[DeadCount - 1];

			uint aliveCount;
			CounterBuffer.InterlockedAdd(0, 1, aliveCount);
			AliveIndexs[aliveCount] = newParticleIndex;

			newPosVelo[newParticleIndex] = (PosVelo)0;
			newPosVelo[newParticleIndex].pos = float4(0, 0, 20, 2);
			int Tweak = aliveCount % 5;
			newPosVelo[newParticleIndex].vel = float4(Tweak, 10, 5, 0);
			newPosVelo[newParticleIndex].pos.y = 20;
			newPosVelo[newParticleIndex].Lifetime = 2;
		}
	}
}


