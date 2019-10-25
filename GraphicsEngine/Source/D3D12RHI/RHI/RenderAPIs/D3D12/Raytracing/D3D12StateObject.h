#pragma once
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../ThirdParty/nv_helpers_dx12/ShaderBindingTableGenerator.h"
#include "Rendering/RayTracing/RHIStateObject.h"
#if WIN10_1809
class DXDescriptor;
class Shader;
class D3D12FrameBuffer;
class Shader_RTBase;

class D3D12StateObject :public RHIStateObject
{
public:
	D3D12StateObject(DeviceContext* D, RHIStateObjectDesc desc);
	~D3D12StateObject();
	virtual void Build() override;

	void AddShaders(CD3DX12_STATE_OBJECT_DESC & Pipe);
	void CreateStateObject();
	void AddHitGroups(CD3DX12_STATE_OBJECT_DESC &RTPipe);
	void AddShaderLibrary(CD3DX12_STATE_OBJECT_DESC &RTPipe, Shader_RTBase* Shader);
	void CreateRootSignatures();
	void CreateLocalRootSigShaders(CD3DX12_STATE_OBJECT_DESC & raytracingPipeline, Shader_RTBase* shader);
	void Trace(const RHIRayDispatchDesc& Desc, RHICommandList * T, D3D12FrameBuffer* target);
	void BuildShaderTables();
	void WriteBinds(Shader_RTBase* shader, std::vector<void *> &Data);
	HighLevelAccelerationStructure* HighLevelStructure = nullptr;
	virtual void RebuildShaderTable() override;
	void BindToList(D3D12CommandList* List);
private:

	ID3D12StateObject* StateObject = nullptr;
	//ID3D12RootSignature* m_raytracingLocalRootSignature = nullptr;
	ID3D12RootSignature* m_raytracingGlobalRootSignature = nullptr;
	ID3D12StateObjectProperties* props = nullptr;
	ID3D12Resource* m_outputResource = nullptr;
	nv_helpers_dx12::ShaderBindingTableGenerator m_sbtHelper;
	ID3D12Resource* m_sbtStorage = nullptr;
	GPUResource* SBTData = nullptr;
	DescriptorGroup* UAVd = nullptr;
	
	uint32_t CurrentSBTSize = 0;
	RHIBuffer* RayDataBuffer = nullptr;
	const int MaxRayDispatchPerFrame = 5;
	int LastFrame = 0;
	int CurrnetIndex = 0;
};
#endif
