#pragma once
#include <map>
#define BUILD_WITH_NVPERFKIT 0
#include <iomanip>
#include <time.h>
#include <vector>


#include "Core/Utils/MovingAverage.h"
#define SCOPE_CYCLE_COUNTER(name) PerfManager::ScopeCycleCounter CYCLECOUNTER(name);
#define SCOPE_CYCLE_COUNTER_GROUP(name,group) PerfManager::ScopeCycleCounter CYCLECOUNTER(name,group);
#define SCOPE_STARTUP_COUNTER(name) PerfManager::ScopeStartupCounter STARTUPCOUNTER(name);
class PerfManager
{
public:
	struct RHI_API ScopeCycleCounter
	{
		ScopeCycleCounter(const char* Name);
		ScopeCycleCounter(const char* Name,const char* group);
		~ScopeCycleCounter();
	private:
		int StatId = -1;
		int GroupID = -1;
	};
	struct RHI_API ScopeStartupCounter
	{
		ScopeStartupCounter(const char* name);
		~ScopeStartupCounter();
	private:
		const char* Name = "";
	};
	CORE_API static long get_nanos();
	static PerfManager* Instance;
	CORE_API static PerfManager* Get();
	static void StartPerfManager();
	static void ShutdownPerfManager();
	PerfManager();
	~PerfManager();
	CORE_API void AddTimer(const char * countername, const char * group);
	CORE_API void AddTimer(int id, int groupid);
	bool InitNV();
	void SampleNVCounters();
	std::string GetCounterData();
	uint64_t GetValue(const char * countername);

	CORE_API static void StartTimer(const char * countername);
	CORE_API static void EndTimer(const char * countername);
	static void StartTimer(int Counterid);
	static void EndTimer(int Counterid);
	static void RenderGpuData(int x, int y);
	float GetAVGFrameRate();
	float GetAVGFrameTime()const;
	std::string GetAllTimers();
	void StartCPUTimer();
	void EndCPUTimer();
	void StartFrameTimer();
	void EndFrameTimer();
	static float GetGPUTime();
	static float GetCPUTime();
	CORE_API static float GetDeltaTime();
	static void SetDeltaTime(float Time);
	static void NotifyEndOfFrame(bool Final = false);
	bool ShowAllStats = false;
	struct TimerData
	{
		float Time = 0.0f;
		MovingAverage* AVG = nullptr;
		std::string name;
		int GroupId = 0;
		bool Active = false;
		int CallCount = 0;
		int LastCallCount = 0;
		bool IsGPUTimer = false;
	};
	CORE_API TimerData* GetTimerData(int id);
	void DrawAllStats(int x, int y, bool IncludeGPUStats = false);
	void UpdateStats();
	void SampleSlowStats();
	void ClearStats();
	void DrawStatsGroup(int x, int & y, std::string GroupFilter, bool IncludeGPU);
	CORE_API void UpdateGPUStat(int id, float newtime);
	CORE_API int GetTimerIDByName(std::string name);
	CORE_API int GetGroupId(std::string name);

	CORE_API void StartSingleActionTimer(std::string Name);
	CORE_API void EndSingleActionTimer(std::string Name);
	void FlushSingleActionTimers();
	void LogSingleActionTimers();
private:
	void Internal_NotifyEndOfFrame();
	void InStartTimer(int targetTimer);
	void InEndTimer(int targetTimer);
	
	class NVAPIManager* NVApiManager = nullptr;
	std::string GetTimerName(int id);
#if BUILD_WITH_NVPERFKIT
	const char * OGLBatch = "OGL batch count";
	const char * OGLMem = "OGL memory allocated";
	const char * OGLTextureMem = "OGL memory allocated (textures)";
	uint64_t hNVPMContext;
	bool DidInitNVCounters = false;
#endif
	std::map<std::string, long> SingleActionTimers;
	std::map<std::string, float> SingleActionTimersAccum;


	std::map<int, long> TimersStartStamps;
	std::map<int, long> TimersEndStamps;
	
	std::map< std::string, int> TimerIDs;
	std::map< std::string, int> GroupIDS;
	std::map<int, float> TimerOutput;
	std::map<int, TimerData> AVGTimers;
	int NextId = 0;
	int NextGroupId = 0;
	const float TimeMS = 1e6f;
	static bool PerfActive;
	float FrameTime = 1.0f;
	float DeltaTime = 0.0f;
	float CPUTime = 0;
	float GPUTime = 0;
	long FrameStart = 0;
	long CPUstart = 0;
	int CurrentFrame = 0;
	int FrameAccum = 0;
	float fpsnexttime = 0;
	float FpsPollSpeed = 0.25f;
	float CurrentAVGFps = 0.0f;
	float AVGFrameTime = 0;
	float FrameTimeAccum = 0.0;
	const bool LockStatsRate = true;
	const float StatsTickRate = (1.0f / 60.0f)*1000.0f;
	const int AvgCount = 50;
	float StatAccum = 0;
	bool Capture = true;
	MovingAverage CPUAVG = MovingAverage(50);
	std::vector<TimerData> SortedTimers;

	//stats UI
	const float TextSize = 0.4f;
	const int Height = 20;
	const int ColWidth = 250;


	const float SlowStatsUpdateRate = 0.25;
	float CurrentSlowStatsUpdate = 0.0f;
};

