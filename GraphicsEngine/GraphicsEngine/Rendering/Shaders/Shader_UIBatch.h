#pragma once
#include "../RHI/Shader.h"
class Shader_UIBatch :
	public Shader
{
	struct UnifromData
	{
		glm::mat4x4 Proj;
	};
public:
	Shader_UIBatch();
	virtual ~Shader_UIBatch();
	void PushTOGPU(CommandListDef * list);
	void UpdateUniforms(glm::mat4x4 Proj);
	class D3D12Shader* GetD3D12Shader();
private:   
	UnifromData data;
	class D3D12CBV* CBV;
};

