static const int PARTICLECOUNTER_OFFSET_ALIVECOUNT = 0;
static const int PARTICLECOUNTER_OFFSET_DEADCOUNT = 4;
static const int PARTICLECOUNTER_OFFSET_EMITCOUNT = 8;
static const int PARTICLECOUNTER_OFFSET_ALIVECOUNT_AFTERSIMULATION = 12;

static const int MAX_PARTICLES = 1000;
struct PosVelo
{
	float4 pos;
	float4 vel;
	float Lifetime;
    float Size;
};