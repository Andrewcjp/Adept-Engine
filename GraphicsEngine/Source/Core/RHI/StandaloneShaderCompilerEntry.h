#pragma once
class StandaloneShaderCompilerEntry
{
public:
	CORE_API StandaloneShaderCompilerEntry(int argc, char *argv[]);
	CORE_API ~StandaloneShaderCompilerEntry();
	CORE_API int Execute();
};

