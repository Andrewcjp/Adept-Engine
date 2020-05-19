#include "HardwareReporter.h"
#include "Core/Utils/StringUtil.h"
#include "RHI/DeviceContext.h"
#ifdef PLATFORM_WINDOWS
std::string GetArchString()
{
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA64
		|| siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		return "x86_64";
	}

	if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
	{
		return "x86";
	}
	if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64)
	{
		return "ARM64";
	}
	if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM)
	{
		return "ARM32";
	}
	return "Unknown";
}

void HardwareReporter::Report()
{
	// Get extended ids.
	int CPUInfo[4] = { -1 };
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	// Get the information associated with each extended ID.
	char CPUBrandString[0x40] = { 0 };
	for (unsigned int i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU brand string and cache information.
		if (i == 0x80000002)
		{
			memcpy(CPUBrandString,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000003)
		{
			memcpy(CPUBrandString + 16,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000004)
		{
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}
	std::string CPUReport = "Hardware Report\nCPU:\n";
	int LengthTarget = 15;
	CPUReport += StringUtils::PadToLength("Name", LengthTarget);
	CPUReport += CPUBrandString;
	CPUReport += "\n";

	ProcessorData data = GetData();
	CPUReport += StringUtils::PadToLength("Core Count", LengthTarget) + std::to_string(data.PhysicalCores) + "/" + std::to_string(data.LogicalCores) + "\n";
	CPUReport += StringUtils::PadToLength("Architecture", LengthTarget) + GetArchString() + "\n";

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);
	CPUReport += StringUtils::PadToLength("Total Memory", LengthTarget) + StringUtils::ByteToGB(statex.ullTotalPhys) + "\n";

	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		CPUReport += "\n";
		CPUReport += RHI::GetDeviceContext(i)->ReportDeviceData();
	}
	CPUReport += "End Hardware Report\n";
	//Log::LogMessage(CPUReport);
	Log::AddToLogFile(CPUReport, true); 
}

size_t NumberOfPhysicalCores() noexcept
{
	DWORD length = 0;
	const BOOL result_first = GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		return 0;
	}

	std::unique_ptr< uint8_t[] > buffer(new uint8_t[length]);
	const PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info =
		reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get());

	const BOOL result_second = GetLogicalProcessorInformationEx(RelationProcessorCore, info, &length);
	if (!result_second)
	{
		return 0;
	}

	size_t nb_physical_cores = 0;
	size_t offset = 0;
	do
	{
		const PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX current_info =
			reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get() + offset);
		offset += current_info->Size;
		++nb_physical_cores;
	} while (offset < length);

	return nb_physical_cores;
}
ProcessorData HardwareReporter::GetData()
{
	SYSTEM_INFO siSysInfo;
	GetSystemInfo(&siSysInfo);
	ProcessorData out;
	out.LogicalCores = siSysInfo.dwNumberOfProcessors;
	out.PhysicalCores = NumberOfPhysicalCores();
	return out;
}
#else
void HardwareReporter::Report()
{}
#endif
