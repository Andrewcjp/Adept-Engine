#pragma once
#include "..\Core\Mesh\MaterialTypes.h"

class Shader_NodeGraph;
class ShaderGraphComplier
{
public:
	ShaderGraphComplier();
	~ShaderGraphComplier();

	Shader_NodeGraph* Complie(MaterialShaderComplieData& Data);

	bool ComplieGraph(MaterialShaderComplieData & data, std::string & ShaderPath);


};

