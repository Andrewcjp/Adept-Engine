#pragma once
//A single test
//Must be run off screen
//Cannot change the current renderer state.
//Cannot tick the GPU.
//on screen is optional and is handled in a special node.
class GPUPerformanceTest
{
public:
	GPUPerformanceTest();
	virtual ~GPUPerformanceTest();

	virtual bool IsTestValid();
	virtual void RunTest();
	void Init(DeviceContext* Context);
	void Destroy();
protected:
	virtual void OnInit();
	virtual void OnDestory();
	DeviceContext* Device = nullptr;
};

