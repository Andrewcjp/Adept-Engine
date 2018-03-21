#pragma once
#include "../RHI/ShaderProgramBase.h"
#include <d3d12.h>
//#include <d3dcompiler.h>
#include "glm\glm.hpp"
//#include <DirectXMath.h>
#include "../EngineGlobals.h"
#include "../RHI/Shader.h"
class D3D12Shader : public ShaderProgramBase
{
public:
	/*__declspec(align(256))*/ struct SceneConstantBuffer//CBV need to be 256 aligned
	{
		glm::mat4 M;
		glm::mat4 V;
		glm::mat4 P;
	};
	struct ShaderBlobs
	{
		ID3DBlob*					vsBlob;
		ID3DBlob*					fsBlob;
		ID3DBlob*					csBlob;
		ID3DBlob*					gsBlob;
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

	static D3D12Shader::PiplineShader CreatePipelineShader(PiplineShader &output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ID3DBlob * vsBlob, ID3DBlob * fsBlob, bool Depthtest);
	static D3D12Shader::PiplineShader CreatePipelineShader(PiplineShader & output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs* blobs, PipeLineState Depthtest);

	inline ID3DBlob*			GetVSBlob()
	{
		return m_vsBlob;
	}
	inline ID3DBlob*			GetFSBlob()
	{
		return m_fsBlob;
	}
	ShaderBlobs* GetShaderBlobs();

	static PiplineShader CreatePipelineShader(D3D12_INPUT_ELEMENT_DESC* inputDisc,  int DescCount, ID3DBlob* vsBlob, ID3DBlob* fsBlob,bool);
	static void CreateRootSig(D3D12Shader::PiplineShader &output,std::vector<Shader::ShaderParameter> Parms);
	static void CreateDefaultRootSig(D3D12Shader::PiplineShader & output);
	void PushCBVToGPU(ID3D12GraphicsCommandList* list,int offset = 0);
	void UpdateCBV(SceneConstantBuffer &buffer, int offset = 0);
	void InitCBV();
	void Init();
	void CreateComputePipelineShader();
	CommandListDef* CreateShaderCommandList(int device = 0);
	ID3D12CommandAllocator* GetCommandAllocator();
	void ResetList(ID3D12GraphicsCommandList * list);
	void SetInputDesc(D3D12_INPUT_ELEMENT_DESC * desc, int size);
	static D3D12_INPUT_ELEMENT_DESC ConvertVertexFormat(Shader::VertexElementDESC * desc);
	ID3DBlob*					m_vsBlob;
	ID3DBlob*					m_fsBlob;
	ID3DBlob*					m_csBlob;
	ID3DBlob*					m_gsBlob;
	PiplineShader m_Shader;
	bool DepthTest = true;
	enum ComputeRootParameters : UINT32
	{
		ComputeRootCBV = 0,
		ComputeRootSRVTable,
		ComputeRootUAVTable,
		ComputeRootParametersCount
	};
	static bool ParseVertexFormat(std::vector<Shader::VertexElementDESC>, D3D12_INPUT_ELEMENT_DESC** Data, int* length);
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
	D3D12_INPUT_ELEMENT_DESC* InputDesc = nullptr;
	ShaderBlobs mBlolbs;
	int Length = 3; 

};

