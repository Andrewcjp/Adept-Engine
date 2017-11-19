#pragma once
#include "../RHI/ShaderProgramBase.h"
#include <d3d12.h>
#include <d3dcompiler.h>
class D3D12Shader : public ShaderProgramBase
{
public:
	D3D12Shader();
	~D3D12Shader();

	// Inherited via ShaderProgramBase
	virtual void CreateShaderProgram() override;
	virtual EShaderError AttachAndCompileShaderFromFile(const char * filename, EShaderType type) override;
	virtual void BuildShaderProgram() override;
	virtual void DeleteShaderProgram() override;
	virtual void ActivateShaderProgram() override;
	virtual void DeactivateShaderProgram() override;
	virtual void SetUniform1UInt(unsigned int value, const char * param) override;
	virtual void SetAttrib4Float(float f1, float f2, float f3, float f4, const char * param) override;
	virtual void BindAttributeLocation(int index, const char * param_name) override;
	inline ID3DBlob*			GetVSBlob()
	{
		return m_vsBlob;
	}

	inline ID3DBlob*			GetFSBlob()
	{
		return m_fsBlob;
	}

private:
	ID3DBlob*					m_vsBlob;
	ID3DBlob*					m_fsBlob;
	int							m_shaderCount;
};

