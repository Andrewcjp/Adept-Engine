#pragma once

class SFRController;
//this has overall control over the frame
class GPULoadBalancer
{
public:
	GPULoadBalancer();
	~GPULoadBalancer();

	static GPULoadBalancer* Get()
	{
		return Instance;
	}
	SFRController* SFR_Controller = nullptr;
	void Tick();
private:
	static GPULoadBalancer* Instance;
};

