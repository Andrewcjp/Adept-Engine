#include "Particle_Common.hlsl"
RWStructuredBuffer<PosVelo> newPosVelo	: register(u0);	// UAV
RWByteAddressBuffer CounterBuffer: register(u1);
StructuredBuffer<uint> AliveIndexs :register(t0);
RWStructuredBuffer<uint> DeadIndexs :register(u2);
RWStructuredBuffer<uint> PostSim_AliveIndex :register(u3);
[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{	
	const float timeStep = 1.0f / 60.0f;
    uint aliveCount = CounterBuffer.Load(PARTICLECOUNTER_OFFSET_ALIVECOUNT);

	if (DTid.x < aliveCount)
	{
        const uint index = AliveIndexs[DTid.x];        
		if (newPosVelo[index].Lifetime > 0.0f)
		{
			const float4 Gravity = float4(0, -9.81, 0, 0);
			newPosVelo[index].vel += Gravity* timeStep;			
            newPosVelo[index].Size = 1.0f;
			newPosVelo[index].pos += float4(newPosVelo[index].vel.xyz, 0.0) * timeStep;
			newPosVelo[index].Lifetime -= timeStep;
			uint NewAliveIndex = 0;
			CounterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION, 1, NewAliveIndex);
			PostSim_AliveIndex[NewAliveIndex] = index;
		}
		else 
		{
			uint DeadParticle;
			CounterBuffer.InterlockedAdd(PARTICLECOUNTER_OFFSET_DEADCOUNT, 1, DeadParticle);
			DeadIndexs[DeadParticle] = index;
		}
	}
}


