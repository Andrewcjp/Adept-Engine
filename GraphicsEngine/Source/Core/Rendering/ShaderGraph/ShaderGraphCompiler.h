#pragma once
#include "..\Core\Mesh\MaterialTypes.h"

class Shader_NodeGraph;
class ShaderGraphCompiler
{
public:
	ShaderGraphCompiler();
	~ShaderGraphCompiler();

	Shader_NodeGraph* Compile(MaterialShaderCompileData& Data);

	bool ComplieGraph(MaterialShaderCompileData & data, std::string & ShaderPath);


};

