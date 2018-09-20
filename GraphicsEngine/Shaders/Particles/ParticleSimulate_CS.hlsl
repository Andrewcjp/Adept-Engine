#include "Particle_Common.hlsl"
RWStructuredBuffer<PosVelo> newPosVelo	: register(u0);	// UAV
RWByteAddressBuffer CounterBuffer: register(u1);
StructuredBuffer<uint> AliveIndexs :register(t2);
RWStructuredBuffer<uint> DeadIndexs :register(u2);
static const int PARTICLECOUNTER_OFFSET_DEADCOUNT = 4;
[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	const uint index = AliveIndexs[DTid.x];
	const float timeStep = 1.0f / 60.0f;
	uint aliveCount = CounterBuffer.Load(0);
	if (DTid.x < aliveCount)
	{
		if (newPosVelo[index].pos.w > 0)
		{
			newPosVelo[index].vel += float4(0, -1, 0, 0.0f)*timeStep;
			newPosVelo[index].pos += float4(newPosVelo[index].vel.xyz, 0.0) * timeStep;
			newPosVelo[index].pos.w -= timeStep;
		}
		else
		{
			if (aliveCount > 0)
			{
				uint DeadParticle;
				CounterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, 1, DeadParticle);
				DeadIndexs[DeadParticle] = index;
				CounterBuffer.InterlockedAdd(0, -1, aliveCount);
			}
		}
	}
}


