#include "CSharpBridgePCH.h"
#include "ManagedBridge.h"
#include "Core/Version.h"
#include "MarshalHelpers.h"

using namespace System::Runtime::InteropServices;

EngineVersion::EngineVersion()
{}


EngineVersion::~EngineVersion()
{}


int EngineVersion::GetVersion()
{
	return ::Version::GetBuildNumber();
}

String^ EngineVersion::GetFullVersionString()
{
	return MarshalHelpers::ConvertString(::Version::GetFullVersionString());
}
