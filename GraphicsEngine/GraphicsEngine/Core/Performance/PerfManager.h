#pragma once
#include <map>
#define BUILD_WITH_NVPERFKIT 0
#include <iomanip>
#include <time.h>
#define STATS 1
class PerfManager
{
public:
	static long get_nanos();
	static PerfManager* Instance;
	static void StartPerfManager();
	PerfManager();
	~PerfManager();
	bool InitNV();
	void SampleNVCounters();
	std::string GetCounterData();
	uint64_t GetValue(const char * countername);

	static void StartTimer(const char * countername);
	static void EndTimer(const char * countername);
	float GetTimerValue(const char * countername);
	float GetAVGFrameRate();
	float GetAVGFrameTime();
	std::string GetAllTimers();
	void StartGPUTimer();
	void EndGPUTimer();
	void StartCPUTimer();
	void EndCPUTimer();
	void StartFrameTimer();
	void EndFrameTimer();
	static float GetGPUTime();
	static float GetCPUTime();
	static float GetDeltaTime();
	bool ShowAllStats = false;
private:
	void InStartTimer(const char * countername);
	void InEndTimer(const char * countername);
	int GetTimerIDByName(std::string name);
	std::string GetTimerName(int id);
#if BUILD_WITH_NVPERFKIT
	const char * OGLBatch = "OGL batch count";
	const char * OGLMem = "OGL memory allocated";
	const char * OGLTextureMem = "OGL memory allocated (textures)";
	uint64_t hNVPMContext;
	bool DidInitNVCounters = false;
#endif
	std::map<int, float> Timers;
	std::map< std::string, int> TimerIDs;
	std::map<int, float> TimerOutput;
	int NextId = 0;
	const float TimeMS = 1e6f;
	static bool PerfActive;
	float FrameTime = 7.0f;
	float CPUTime = 0;
	float GPUTime = 0;
#if BUILD_OPENGL
	GLint stopTimerAvailable = 0;
#endif
	bool WaitGPUTimerQuerry = false;
	unsigned int queryID[2];
	long FrameStart = 0;
	long CPUstart = 0;
	int CurrentFrame = 0;
	int FrameAccum = 0;
	float fpsnexttime = 0;
	float FpsPollSpeed = 0.25f;
	float CurrentAVGFps;
	float AVGFrameTime = 0;
	float FrameTimeAccum = 0.0;
	
	float StatsUpdateSpeed = 100.0f;
	float StatAccum = 0;
	bool Capture = true;
	//GPU Bottleneck,2809,simexp,raw,KEPLER_CPU,GRAPHICS,GPU,Bottleneck,UINT64,"Counter ID for GPU bottleneck"
};

