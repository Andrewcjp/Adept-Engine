#pragma once
#include <map>
#define BUILD_WITH_NVPERFKIT 0
class PerfManager
{
public:
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
	std::string GetAllTimers();
private:
	void InStartTimer(const char * countername);
	void InEndTimer(const char * countername);
	int GetTimerIDByName(std::string name);
	std::string GetTimerName(int id);
	const char * OGLBatch = "OGL batch count";
	const char * OGLMem = "OGL memory allocated";
	const char * OGLTextureMem = "OGL memory allocated (textures)";
	//	const char * OGLBatch = "OGL batch count";
	uint64_t hNVPMContext;
	bool DidInitNVCounters = false;
	std::map<int, float> Timers;
	std::map< std::string, int> TimerIDs;
	std::map<int, float> TimerOutput;
	int NextId = 0;
	float TimeMS = 1e6f;
	static bool PerfActive;
	//GPU Bottleneck,2809,simexp,raw,KEPLER_CPU,GRAPHICS,GPU,Bottleneck,UINT64,"Counter ID for GPU bottleneck"
};

