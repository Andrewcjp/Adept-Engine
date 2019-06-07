#pragma once
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../ThirdParty/nv_helpers_dx12/ShaderBindingTableGenerator.h"
#include "Rendering/RayTracing/RHIStateObject.h"

class Descriptor;
class Shader;
class D3D12FrameBuffer;
namespace GlobalRootSignatureParams
{
	enum Value
	{
		OutputViewSlot = 0,
		AccelerationStructureSlot,
		CameraBuffer,
		Count
	};
}

namespace LocalRootSignatureParams
{
	enum Value
	{
		CubeConstantSlot = 0,
		Count
	};
}
struct RTCameraData
{
	glm::mat4x4 IView;
	glm::mat4x4 IProj;
};
class D3D12StateObject :public RHIStateObject
{
public:
	D3D12StateObject(DeviceContext* D);
	~D3D12StateObject();
	virtual void Build() override;

	void CreateStateObject();

	void CreateRootSignatures();
	void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC * raytracingPipeline);
	void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC & desc, ID3D12RootSignature ** rootSig);
	void CreateRaytracingOutputBuffer();
	void Trace(const RHIRayDispatchDesc& Desc,RHICommandList * T,D3D12FrameBuffer* target);
	void BuildShaderTables();
	HighLevelAccelerationStructure* High = nullptr;
	RTCameraData Data;
private:
	
	ID3D12StateObject* StateObject = nullptr;
	ID3D12RootSignature* m_raytracingLocalRootSignature = nullptr;
	ID3D12RootSignature* m_raytracingGlobalRootSignature = nullptr;
	ID3D12StateObjectProperties* props;
	ID3D12Resource* m_outputResource;
	nv_helpers_dx12::ShaderBindingTableGenerator m_sbtHelper;
	ID3D12Resource* m_sbtStorage;
	DescriptorGroup* UAVd = nullptr;
	D3D12Buffer* CBV = nullptr;
};

