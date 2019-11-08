#pragma once
#include "RHI/Shader.h"
class Shader_Line :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER_ARGS(Shader_Line, bool);

	Shader_Line(DeviceContext* device, bool is2D);
	virtual ~Shader_Line();
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;

	virtual const std::string GetName() override;
	void SetParameters(RHICommandList* List, RHIBuffer * UBO);
private:
	bool IsTwo = false;
};

