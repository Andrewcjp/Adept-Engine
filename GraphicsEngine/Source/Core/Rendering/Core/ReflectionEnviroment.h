#pragma once

class RelfectionProbe;
class RHICommandList;
class ReflectionEnviroment
{
public:
	ReflectionEnviroment();
	~ReflectionEnviroment();
	///update all probes etc.
	void Update();
	void UpdateRelflectionProbes(RHICommandList * commandlist);
	bool AnyProbesNeedUpdate();
	void RenderCubemap(RelfectionProbe * Map, RHICommandList * commandlist);

private:
	std::vector<RelfectionProbe*> Probes;

};

