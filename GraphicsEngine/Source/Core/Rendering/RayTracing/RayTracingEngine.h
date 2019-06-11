#pragma once

class ShaderBindingTable;
class RayTracingCommandList;
//This handles render interaction with RT features
//It abstracts between the Software and API support modes and handles build AS's etc.
//it will handle different types of Acceleration structure for multiple types of devices e.g. RTX GPU + Software RT GPU. 
class RayTracingEngine
{
public:
	RayTracingEngine();
	~RayTracingEngine();
	static RayTracingEngine* Get();
	void EnqueueForBuild(LowLevelAccelerationStructure* Struct);
	void EnqueueForBuild(HighLevelAccelerationStructure* Struct);
	//call as early as possible to start on async compute
	void BuildForFrame(RHICommandList * List);
	//used in rendering to launch rays
	void DispatchRaysForMainScenePass(FrameBuffer* Target);
	void SetShaderTable(ShaderBindingTable * SBT);
	void OnFirstFrame();

	void BuildStructures();
	RayTracingCommandList* CreateRTList(DeviceContext* Device);
	void TraceRaysForReflections(FrameBuffer * Target, FrameBuffer* NormalSrcBuffer);
private:
	std::vector<LowLevelAccelerationStructure*> LASToBuild;
	std::vector<HighLevelAccelerationStructure*> HASToBuild;
	HighLevelAccelerationStructure* CurrnetHL = nullptr;
	RHICommandList* AsyncbuildList = nullptr;
	RHIStateObject* StateObject = nullptr;
	bool Build = false;
	RayTracingCommandList* RTList = nullptr;
	ShaderBindingTable* DefaultTable = nullptr;
};

