#include "Particle_Common.hlsl"
cbuffer emitData: register(b0)
{
	int EmitCount;
};
RWStructuredBuffer<PosVelo> newPosVelo	: register(u0);	// UAV
RWByteAddressBuffer CounterBuffer: register(u1);
RWStructuredBuffer<uint> AliveIndexs :register(u2);
RWStructuredBuffer<uint> DeadIndexs :register(u3);
static const int PARTICLECOUNTER_OFFSET_DEADCOUNT = 4;
[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	if (EmitCount < 0)
	{
		return;
	}
	if (DTid.x <= EmitCount)
	{
		uint FirstAliveCount = CounterBuffer.Load(0);
		uint newParticleIndex = 0;
		if (FirstAliveCount > 100)
		{
			//newParticleIndex = AliveIndexs[0];
		}
		else
		{
			uint DeadParticle;
			CounterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, -1, DeadParticle);
			newParticleIndex = DeadIndexs[DeadParticle - 1];

			uint aliveCount;
			CounterBuffer.InterlockedAdd(0, 1, aliveCount);
			AliveIndexs[aliveCount] = newParticleIndex;
		}

		newPosVelo[newParticleIndex] = (PosVelo)0;
		newPosVelo[newParticleIndex].pos = float4(0, 0, 0, 5);
		newPosVelo[newParticleIndex].vel = float4(0, 0, 0, 0);
		newPosVelo[newParticleIndex].pos.y = 20;
	}
}


