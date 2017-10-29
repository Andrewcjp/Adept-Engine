#pragma once
#include "EngineGlobals.h"
#if BUILD_D3D11
#include <vector>
#include "../RHI/ShaderBase.h"
#include "../RHI/ShaderProgramBase.h"
#include <d3d11_1.h>
#include <d3d11.h>
#include <d3dcompiler.h>
class D3D11ShaderProgram :public ShaderProgramBase
{
private:
	ID3D11VertexShader*			m_vertexShader;
	ID3D11PixelShader*			m_fragmentShader;
	ID3DBlob*					m_vsBlob;
	ID3DBlob*					m_fsBlob;
	ID3D11DeviceContext*		m_context;
	ID3D11Device*				m_device;
	int							m_shaderCount;

public:
	ID3D11ShaderReflection* pReflector;
	D3D11ShaderProgram()
	{
		m_device = NULL;
		m_context = NULL;
		m_vertexShader = NULL;
		m_fragmentShader = NULL;
		m_vsBlob = NULL;
		m_fsBlob = NULL;
	}

	D3D11ShaderProgram(ID3D11Device* dev, ID3D11DeviceContext* cntx)
	{
		m_device = dev;
		m_context = cntx;
		m_vertexShader = NULL;
		m_fragmentShader = NULL;
		m_vsBlob = NULL;
		m_fsBlob = NULL;
	}

	virtual						~D3D11ShaderProgram();

	inline void					SetContext(ID3D11DeviceContext* cntx)
	{
		m_context = cntx;
	}

	inline void					SetDevice(ID3D11Device* dev)
	{
		m_device = dev;
	}

	inline ID3DBlob*			GetVSBlob()
	{
		return m_vsBlob;
	}

	inline ID3DBlob*			GetFSBlob()
	{
		return m_fsBlob;
	}

	inline ID3D11Device*		GetDevice()
	{
		return m_device;
	}

	inline ID3D11DeviceContext*	GetContext()
	{
		return m_context;
	}

	EShaderError				AttachAndCompileShaderFromFile(const char* filename, EShaderType type)override;
	void						DeleteShaderProgram();
	void						ActivateShaderProgram();
	void						DeactivateShaderProgram();

	// Inherited via ShaderProgramBase
	virtual void CreateShaderProgram() override;
	virtual void BuildShaderProgram() override;
	virtual void SetUniform1UInt(unsigned int value, const char * param) override;
	virtual void SetAttrib4Float(float f1, float f2, float f3, float f4, const char * param) override;
	virtual void BindAttributeLocation(int index, const char * param_name) override;
};
#endif