#pragma once
#include "RHI/ShaderBase.h"
#define CSO_VERSION 3
class ShaderProgramBase;
struct ShaderByteCodeBlob;
struct ShaderComplieItem;
class ShaderCache
{
public:
	ShaderCache();
	~ShaderCache();
	RHI_API static ShaderByteCodeBlob * GetShader(ShaderComplieItem * item);
	ShaderByteCodeBlob * IN_GetShader(ShaderComplieItem * item);
	void MirrorShaderToBuiltPlat(ShaderComplieItem * item, EPlatforms::Type platform);
	RHI_API static ShaderCache* Get();
	RHI_API static const std::string GetShaderInstanceHash(ShaderComplieItem* shader);
	RHI_API static const std::string GetShaderNamestr(const std::string & Shadername, const std::string & InstanceHash, EShaderType::Type type);
	void PrintShaderStats();
private:
	bool TryLoadCachedShader(const std::string & Name, ShaderComplieItem * Blob, const std::string & InstanceHash, EShaderType::Type type,EPlatforms::Type platform  = EPlatforms::Limit);
	void WriteBlobToFile(ShaderComplieItem* item, EPlatforms::Type platform = EPlatforms::Limit);
	static ShaderCache* Instance;
	struct CSOHeader
	{
		uint64 Version = CSO_VERSION;
		uint64 Size = 0;
	};
#if !BUILD_SHIPPING
	struct ShaderStats
	{
		int ShaderComplieCount = 0;
		int TotalShaderCount = 0;
		int ShaderLoadFromCacheCount = 0;
	};
	ShaderStats stats;
#endif
};

