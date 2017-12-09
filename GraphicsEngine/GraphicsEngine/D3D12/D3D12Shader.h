#pragma once
#include "../RHI/ShaderProgramBase.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include "glm\glm.hpp"
#include <DirectXMath.h>
#include "../EngineGlobals.h"
class D3D12Shader : public ShaderProgramBase
{
public:
	/*__declspec(align(256))*/ struct SceneConstantBuffer//CBV need to be 256 aligned
	{
		glm::mat4 M;
		glm::mat4 V;
		glm::mat4 P;
	};

	D3D12Shader();
	~D3D12Shader();
	struct PiplineShader
	{
		ID3D12PipelineState* m_pipelineState;
		ID3D12RootSignature* m_rootSignature;
	};
	// Inherited via ShaderProgramBase
	virtual void CreateShaderProgram() override;
	virtual EShaderError AttachAndCompileShaderFromFile(const char * filename, EShaderType type) override;
	virtual void BuildShaderProgram() override;
	virtual void DeleteShaderProgram() override;
	virtual void ActivateShaderProgram() override;
	void ActivateShaderProgram(ID3D12GraphicsCommandList * list);
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
	PiplineShader CreatePipelineShader(D3D12_INPUT_ELEMENT_DESC* inputDisc,  int DescCount, ID3DBlob* vsBlob, ID3DBlob* fsBlob);
	void PushCBVToGPU(ID3D12GraphicsCommandList* list,int offset = 0);
	void UpdateCBV(SceneConstantBuffer &buffer, int offset = 0);
	void InitCBV();
	void Init();
	CommandListDef* CreateShaderCommandList();
	ID3D12CommandAllocator* GetCommandAllocator();
	ID3DBlob*					m_vsBlob;
	ID3DBlob*					m_fsBlob;
	PiplineShader m_Shader;
private:
	int							m_shaderCount;
	int							InitalBufferCount = 50;
	int							CB_Size;
	class D3D12CBV* CBV;
	ID3D12Resource* m_constantBuffer;
	SceneConstantBuffer m_constantBufferData;
	UINT8* m_pCbvDataBegin;
	ID3D12DescriptorHeap* m_cbvHeap;
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12DescriptorHeap* m_samplerHeap;

};

