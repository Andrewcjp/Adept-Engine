#pragma once
namespace FileUtils { class CSVWriter; }

namespace EBenchMarkerMode
{
	enum Type
	{
		Off, //no data is captured
		RunningBench,//the Game is only running the benchmark
		Capturing,//capturing performance data to a log file
		Limit
	};
};

namespace ECoreStatName
{
	enum Type
	{
		GPU,
		CPU,
		FrameTime,
		FrameRate,
		Limit
	};
};

class BenchMarker
{
public:
	BenchMarker();
	~BenchMarker();
	//Control Functions
	void StartBenchMark();
	void StopBenchMark();
	void StartCapture();
	void EndCapture();

	//For PerfManager
	void TickBenchMarker();
	void WriteStat(int statid, float value);
	void WriteCoreStat(ECoreStatName::Type stat, float value);
private:
	EBenchMarkerMode::Type CurrentMode = EBenchMarkerMode::Off;
	void CapturePerfMarkers();
	
	void WriteSummaryToDisk();
	std::string GetCoreTimerSummary(ECoreStatName::Type CoreStat);
	std::string GetTimerSummary(std::string statname);
	std::string GetTimerSummary(int Statid);
	void WriteFullStatsHeader(bool OnlyCoreStats);
	void WriteCSV(bool OnlyCoreStats);
	//This contains a single stat and all its data for the duration of the stat collection
	struct PerformanceLogStat
	{
		PerformanceLogStat()
		{}
		PerformanceLogStat(std::string statname)
		{
			name = statname;
		}
		std::string name = "";
		int id = 0;
		void AddData(float t)
		{
			Data.push_back(t);
		}
		std::vector<float>& GetData()
		{
			return Data;
		}
	private:
		std::vector<float> Data;
	};
	FileUtils::CSVWriter* CSV = nullptr;
	std::string ProcessTimerData(PerformanceLogStat * PLS);
	std::map<int, PerformanceLogStat*> StatLogs;
	const int DiskFlushRate = 100;
	std::string SummaryOutputFileName = "";
	PerformanceLogStat* CoreStats[ECoreStatName::Limit];
};

