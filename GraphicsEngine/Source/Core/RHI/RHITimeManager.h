#pragma once

#include "RHI_inc_fwd.h"

#include <unordered_map>
#define SEPERATE_RAYTRACING_TIMERS 0
#define PERDRAWTIMING 0
class MovingAverage;
//GPU timers are handled as a special case
struct GPUTimer
{
	std::vector<uint64_t> Stamps;
	
	std::string name;
	DeviceContext* Context = nullptr;
	ECommandListType::Type Type = ECommandListType::Limit;
	RHI_API void Clear();
	RHI_API void AddResults(uint64_t start);
	std::vector<MovingAverage*> NormalisedStamps;
	void Normalise(uint64_t StartStamp,uint64_t endstamp);
};
struct GPUTimerPair
{
	float Stamps[2];
	GPUTimer* Owner = nullptr;
	int Offset = 0;
};
class  RHITimeManager
{
public:
	RHI_API RHITimeManager(class DeviceContext* context);
	RHI_API virtual ~RHITimeManager();
	RHI_API virtual void UpdateTimers() = 0;
	RHI_API virtual void SetTimerName(int index, std::string Name, ECommandListType::Type type) = 0;
	RHI_API virtual void StartTotalGPUTimer(RHICommandList * ComandList) = 0;
	RHI_API virtual void StartTimer(RHICommandList * ComandList, int index) = 0;
	RHI_API virtual void EndTimer(RHICommandList * ComandList, int index) = 0;
	RHI_API virtual void EndTotalGPUTimer(RHICommandList * ComandList) = 0;
	RHI_API virtual float GetTotalTime() = 0;
	RHI_API int GetGPUTimerId(std::string name);
	RHI_API GPUTimer* GetTimer(std::string name, DeviceContext* D);
	RHI_API GPUTimer* GetOrCreateTimer(std::string name, DeviceContext* D, ECommandListType::Type type);
	RHI_API std::string GetTimerName(int id);
	std::vector<GPUTimerPair*> GetGPUTimers(ECommandListType::Type type);
	RHI_API bool IsRunning();
protected:
	RHI_API void PushToPerfManager();
	void AddTimer(GPUTimer* Data, std::string name, DeviceContext* D);
	DeviceContext * Context = nullptr;
	const int CopyOffset = EGPUTIMERS::LIMIT;
	std::map<std::string, int> GPUTimerMap;
	int AssignedIdCount = EGPUTIMERS::LIMIT + EGPUCOPYTIMERS::LIMIT - 1;
	std::vector<std::string> TimerNames;
	std::map<std::pair<std::string, DeviceContext*>, GPUTimer*> Timers;
};

struct ScopedGPUTimer
{
public:
	ScopedGPUTimer(RHICommandList* List, std::string Name, bool PerDraw = false);
	~ScopedGPUTimer();
private:
	int GPUTimerId = 0;
	RHICommandList* cmdlist = nullptr;
};
#if !BUILD_SHIPPING
#define DECALRE_SCOPEDGPUCOUNTER(list,name) ScopedGPUTimer PREPROCESSOR_JOIN(COUNTER,__LINE__)(list,name);
#else
#define DECALRE_SCOPEDGPUCOUNTER(list,name)
#endif
#if PERDRAWTIMING
#define PERDRAWTIMER(list,name) ScopedGPUTimer PREPROCESSOR_JOIN(COUNTER,__LINE__)(list,name,true);
#else
#define PERDRAWTIMER(list,name) 
#endif