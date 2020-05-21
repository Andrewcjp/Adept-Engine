#include "ShaderCache.h"
#include "RHI/ShaderProgramBase.h"
#include "Core/Utils/FileUtils.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/ShaderPreProcessor.h"
#include "Core/Assets/ShaderComplier.h"
#include "RHI/ShaderComplierModule.h"
#include <ios>
#include<iostream>
#include<fstream>
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/BinaryArchive.h"
static ConsoleVariable NoShaderCache("NoShaderCache", 0, ECVarType::LaunchOnly);
static ConsoleVariable MirrorShaders("MirrorShaders", 0, ECVarType::LaunchOnly);
ShaderCache* ShaderCache::Instance = nullptr;
const std::string ShaderCache::GetShaderInstanceHash(ShaderComplieItem* shader)
{
	if (shader->Defines.size() == 0)
	{
		return "";
	}
	std::string DefineSum;
	struct less_than_key
	{
		inline bool operator() (const ShaderProgramBase::Shader_Define& struct1, const ShaderProgramBase::Shader_Define& struct2)
		{
			return (struct1.Name < struct2.Name);
		}
	};
	std::sort(shader->Defines.begin(), shader->Defines.end(), less_than_key());
	for (ShaderProgramBase::Shader_Define d : shader->Defines)
	{
		DefineSum += d.Name + d.Value;
	}
	size_t Hash = std::hash<std::string>{} (DefineSum);
	return "_" + std::to_string(Hash);
}


ShaderCache::ShaderCache()
{
	if (NoShaderCache.GetBoolValue())
	{
		Log::LogMessage("Shader Cache Disabled", Log::Warning);
	}
	MirrorShaders.SetValue(true);
}


ShaderCache::~ShaderCache()
{}

ShaderByteCodeBlob* ShaderCache::GetShader(ShaderComplieItem* item)
{
	return Get()->IN_GetShader(item);
}

ShaderByteCodeBlob* ShaderCache::IN_GetShader(ShaderComplieItem* item)
{
#if defined(PLATFORM_WINDOWS) && WITH_EDITOR
	if (MirrorShaders.GetBoolValue())
	{
		for (int i = EPlatforms::Android+1; i < EPlatforms::Limit; i++)
		{
			MirrorShaderToBuiltPlat(item, (EPlatforms::Type)i);
			item->ResetOutput();
		}
	}
#endif
	if (TryLoadCachedShader(item->ShaderName, item, GetShaderInstanceHash(item), item->Stage,PlatformApplication::GetPlatform()))
	{
		item->CacheHit = true;
		return item->Blob;
	}
	ShaderComplier::Get()->ComplieShaderNew(item, PlatformApplication::GetPlatform());
	WriteBlobToFile(item,PlatformApplication::GetPlatform());
	return item->Blob;
}

void ShaderCache::MirrorShaderToBuiltPlat(ShaderComplieItem* item, EPlatforms::Type platform)
{
	if (item->Stage == EShaderType::SHADER_RT_LIB && (platform != EPlatforms::Windows || platform != 5))
	{
		return;
	}
	if (TryLoadCachedShader(item->ShaderName, item, GetShaderInstanceHash(item), item->Stage, platform))
	{
		return;
	}
	item->TargetPlatfrom = platform;
	ShaderComplier::Get()->ComplieShaderNew(item, platform);
	if (item->Result == EShaderError::SHADER_ERROR_UNSUPPORTED)
	{
		return;
	}
	WriteBlobToFile(item, platform);
}

ShaderCache * ShaderCache::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ShaderCache();
	}
	return Instance;
}
const std::string ShaderCache::GetShaderNamestr(const std::string & Shadername, const std::string & InstanceHash, EShaderType::Type type)
{
	std::string OutputName = Shadername;
	OutputName += "_" + std::to_string((int)type);
	OutputName += "_" + InstanceHash;
	if (ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		OutputName += "_D";
	}
	OutputName += "_DIXL";
	OutputName += ".cso";
	return OutputName;
}

bool ShaderCache::TryLoadCachedShader(const std::string& Name, ShaderComplieItem* Item, const std::string & InstanceHash, EShaderType::Type type, EPlatforms::Type platform)
{
	SCOPE_STARTUP_COUNTER("Shader Read");

	const std::string FullShaderName = GetShaderNamestr(Name, InstanceHash, type);
	std::string ShaderPath = AssetManager::GetShaderCacheDir(platform) + FullShaderName;

#if BUILD_PACKAGE
	ensureFatalMsgf(FileUtils::File_ExistsTest(ShaderPath), "Missing shader: " + FullShaderName);
	bool CSOValid = true;
#else
	bool CSOValid = ShaderPreProcessor::CheckCSOValid(Name, FullShaderName, platform);
#endif
	if (FileUtils::File_ExistsTest(ShaderPath) && CSOValid && !NoShaderCache.GetBoolValue())
	{
		CSOHeader header;
		ShaderByteCodeBlob* newblob = new ShaderByteCodeBlob();
		BinaryArchive Arch;
		Arch.Open(ShaderPath);
		Arch.LinkHeader(header);
		if (header.Version != CSO_VERSION)
		{
			AD_ERROR("CSO version mismatch");
			return false;
		}
		newblob->ByteCode = malloc(header.Size);
		Arch.LinkData(newblob->ByteCode, header.Size);
		newblob->Length = header.Size;
		Arch.LinkVector(Item->Data->RootConstants);
		Arch.Close();
		Item->Blob = newblob;
		return true;
	}
	Log::LogMessage("Recompile triggered for " + Name + " on Platfrom: " + EPlatforms::ToString(platform));
	return false;

}
void ShaderCache::WriteBlobToFile(ShaderComplieItem* item, EPlatforms::Type platform)
{
	if (item->Result != EShaderError::SHADER_ERROR_NONE)
	{
		return;
	}
	FileUtils::CreateDirectoriesToFullPath(AssetManager::GetShaderCacheDir(platform) + item->ShaderName + ".");
	const std::string FullShaderName = GetShaderNamestr(item->ShaderName, GetShaderInstanceHash(item), item->Stage);
	std::string path = AssetManager::GetShaderCacheDir(platform) + FullShaderName;
	CSOHeader header;
	header.Version = CSO_VERSION;
	header.Size = item->Blob->Length;

	BinaryArchive Arch;
	Arch.Open(path, true);
	Arch.LinkHeader(header);
	Arch.LinkData(item->Blob->ByteCode, header.Size);
	Arch.LinkVector(item->Data->RootConstants);
	Arch.Close();
}