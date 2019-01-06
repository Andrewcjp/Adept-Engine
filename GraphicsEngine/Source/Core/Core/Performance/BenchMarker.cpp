
#include "BenchMarker.h"
#include "PerfManager.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Utils/FileUtils.h"
#include "RHI/RHI.h"


BenchMarker::BenchMarker()
{
	CoreStats[ECoreStatName::FrameTime] = new PerformanceLogStat("Frame");
	CoreStats[ECoreStatName::FrameRate] = new PerformanceLogStat("Frame Rate");
	CoreStats[ECoreStatName::CPU] = new PerformanceLogStat("CPU");
	CoreStats[ECoreStatName::GPU] = new PerformanceLogStat("GPU");
	StartCapture();
}

BenchMarker::~BenchMarker()
{
	WriteSummaryToDisk();
}

void BenchMarker::TickBenchMarker()
{
	if (CurrentMode == EBenchMarkerMode::Off || CurrentMode == EBenchMarkerMode::Limit)
	{
		return;
	}
	CapturePerfMarkers();
}

void BenchMarker::StartBenchMark()
{
	StartCapture();
	CurrentMode = EBenchMarkerMode::RunningBench;
}

void BenchMarker::StopBenchMark()
{
	EndCapture();
}

void BenchMarker::StartCapture()
{
	CurrentMode = EBenchMarkerMode::Capturing;
	SummaryOutputFileName = AssetManager::GetGeneratedDir() + "\\PerfLog.txt";
	CSV = new FileUtils::CSVWriter(AssetManager::GetGeneratedDir() + "\\PerfData.csv");
}

void BenchMarker::EndCapture()
{
	CurrentMode = EBenchMarkerMode::Off;
	WriteSummaryToDisk();
	WriteCSV(false);
}

void BenchMarker::WriteStat(int statid, float value)
{
	BenchMarker::PerformanceLogStat* stat = nullptr;
	std::map<int, PerformanceLogStat*>::iterator finditor = StatLogs.find(statid);
	if (finditor != StatLogs.end())
	{
		stat = finditor->second;
		stat->AddData(value);
	}
	else
	{
		stat = new BenchMarker::PerformanceLogStat();
		stat->id = statid;
		stat->name = PerfManager::Get()->GetTimerData(statid)->name;
		stat->AddData(value);
		StatLogs.emplace(statid, stat);
	}
}

void BenchMarker::WriteCoreStat(ECoreStatName::Type stat, float value)
{
	if (CoreStats[stat] != nullptr)
	{
		CoreStats[stat]->AddData(value);
	}
}

void BenchMarker::CapturePerfMarkers()
{
	PerfManager::Get()->WriteLogStreams(this);
}

void BenchMarker::WriteFullStatsHeader(bool OnlyCoreStats)
{
	for (int id = 0; id < ECoreStatName::Limit; id++)
	{
		CSV->AddEntry(CoreStats[id]->name);
	}
	if (!OnlyCoreStats)
	{
		for (std::map<int, PerformanceLogStat*>::iterator it = StatLogs.begin(); it != StatLogs.end(); ++it)
		{
			//write the header data
			CSV->AddEntry(it->second->name);
		}
	}
	CSV->AddLineBreak();
}

void BenchMarker::WriteSummaryToDisk()
{
	std::string summary = "";
	summary += GetCoreTimerSummary(ECoreStatName::FrameRate);
	summary.append("\n");
	summary += GetCoreTimerSummary(ECoreStatName::FrameTime);
	summary.append("\n");
	summary += GetCoreTimerSummary(ECoreStatName::CPU);
	summary.append("\n");
	summary += GetCoreTimerSummary(ECoreStatName::GPU);
	summary.append("\n");
	FileUtils::WriteToFile(SummaryOutputFileName, summary);
}

std::string BenchMarker::GetCoreTimerSummary(ECoreStatName::Type CoreStat)
{
	return ProcessTimerData(CoreStats[CoreStat]);
}

std::string BenchMarker::GetTimerSummary(std::string statname)
{
	return GetTimerSummary(PerfManager::Get()->GetTimerIDByName(statname));
}

std::string BenchMarker::GetTimerSummary(int Statid)
{
	std::string output = "";
	std::map<int, PerformanceLogStat*>::iterator finditor = StatLogs.find(Statid);
	if (finditor == StatLogs.end())
	{
		output = "No data";
		return output;
	}
	return ProcessTimerData(finditor->second);
}

void BenchMarker::WriteCSV(bool OnlyCoreStats)
{
	CSV->Clear();
	WriteFullStatsHeader(OnlyCoreStats);
	int LongestStatArray = 1;
	for (int i = 0; i < LongestStatArray; i++)
	{
		for (int id = 0; id < ECoreStatName::Limit; id++)
		{
			if (i < CoreStats[id]->GetData().size())
			{
				LongestStatArray = std::max(LongestStatArray, (int)CoreStats[id]->GetData().size());
				CSV->AddEntry(std::to_string(CoreStats[id]->GetData()[i]));
			}
		}
		if (!OnlyCoreStats)
		{
			for (std::map<int, PerformanceLogStat*>::iterator it = StatLogs.begin(); it != StatLogs.end(); ++it)
			{
				//write the header data
				if (i < it->second->GetData().size())
				{
					CSV->AddEntry(std::to_string(it->second->GetData()[i]));
					LongestStatArray = std::max(LongestStatArray, (int)it->second->GetData().size());
				}
			}
		}
		CSV->AddLineBreak();
	}
	CSV->Save();
}

std::string BenchMarker::ProcessTimerData(PerformanceLogStat* PLS)
{
	std::vector<float> data = PLS->GetData();
	float AVG = 0;
	float Max = 0;
	float Min = FLT_MAX;
	for (int i = 0; i < data.size(); i++)
	{
		if (i == 0)
		{
			continue;
		}
		AVG += data[i];
		Max = glm::max(Max, data[i]);
		Min = fminf(Min, data[i]);
	}
	AVG /= data.size();
	std::stringstream stream;
	stream << std::fixed << std::setprecision(3) << "Name: " << PLS->name << " AVG: " << AVG << " Min: " << Min << " Max: " << Max << " ";
	return stream.str();
}

