#include "D3D12StateObject.h"
#include "RHI/RenderAPIs/D3D12/D3D12DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../ThirdParty/DXRHelper.h"
#include "../D3D12CommandList.h"
#include "D3D12HighLevelAccelerationStructure.h"
#include "../DescriptorHeapManager.h"
#include "../Descriptor.h"
#include "Rendering/RayTracing/RHIStateObject.h"
#include "RHI/Shader.h"
#include "../D3D12Shader.h"
#include "../DescriptorGroup.h"
#include "../D3D12Framebuffer.h"


const wchar_t* c_hitGroupName = L"MyHitGroup";
const wchar_t* c_raygenShaderName = L"rayGen";
const wchar_t* c_closestHitShaderName = L"chs";
const wchar_t* c_missShaderName = L"miss";

D3D12StateObject::D3D12StateObject(DeviceContext* D) :RHIStateObject(D)
{}


D3D12StateObject::~D3D12StateObject()
{}

void D3D12StateObject::Build()
{
	CBV = new D3D12Buffer(ERHIBufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(Data), 1);
	//RT = new Shader_RTBase();
	//RT->GetShaderProgram();
	CreateRootSignatures();
	CreateStateObject();
	BuildShaderTables();
	CreateRaytracingOutputBuffer();
	Log::LogMessage("DXR State Object built");
}

void D3D12StateObject::CreateStateObject()
{
	//CD3D12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
	CD3DX12_STATE_OBJECT_DESC RTPipe;
	RTPipe.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
	auto lib = RTPipe.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	IDxcBlob* B = ((D3D12Shader*)Target->GetShaderProgram())->GetShaderBlobs()->RTLibBlob;
	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(B->GetBufferPointer(), B->GetBufferSize());
	lib->SetDXILLibrary(&libdxil);


	lib->DefineExport(c_raygenShaderName);
	lib->DefineExport(c_closestHitShaderName);
	lib->DefineExport(c_missShaderName);

	auto hitGroup = RTPipe.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
	hitGroup->SetHitGroupExport(c_hitGroupName);
	hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

	auto shaderConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = sizeof(glm::vec4);    // float4 pixelColor
	UINT attributeSize = sizeof(glm::vec2);  // float2 barycentrics
	shaderConfig->Config(payloadSize, attributeSize);

	CreateLocalRootSignatureSubobjects(&RTPipe);

	auto globalRootSignature = RTPipe.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature);

	auto pipelineConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	// PERFOMANCE TIP: Set max recursion depth as low as needed 
	// as drivers may apply optimization strategies for low recursion depths.
	UINT maxRecursionDepth = 1; // ~ primary rays only. 
	pipelineConfig->Config(maxRecursionDepth);

	D3D12RHI::GetDXCon(RHI::GetDefaultDevice())->GetDevice5()->CreateStateObject(RTPipe, IID_PPV_ARGS(&StateObject));
}

void D3D12StateObject::CreateRootSignatures()
{
	// Global Root Signature
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	{
		CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Performance TIP: Order from most frequent to least frequent.
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture

		CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count];
		rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(1, &ranges[0]);
		rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);
		rootParameters[GlobalRootSignatureParams::CameraBuffer].InitAsConstantBufferView(0);
		CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
		SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
	}

	// Local Root Signature
	// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	{
		CD3DX12_ROOT_PARAMETER rootParameters[LocalRootSignatureParams::Count];
		rootParameters[LocalRootSignatureParams::CubeConstantSlot].InitAsConstants(32, 1);
		CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
		localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature);
	}
}
void D3D12StateObject::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
	// Ray gen and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

	// Local root signature to be used in a hit group.
	auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	localRootSignature->SetRootSignature(m_raytracingLocalRootSignature);
	// Define explicit shader association for the local root signature. 
	{
		auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		rootSignatureAssociation->AddExport(c_hitGroupName);
	}
}

void D3D12StateObject::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ID3D12RootSignature** rootSig)
{
	ID3DBlob* blob;
	ID3DBlob* error;
	ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error));
	ThrowIfFailed(D3D12RHI::GetDXCon(RHI::GetDefaultDevice())->GetDevice()->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(rootSig)));
}

void D3D12StateObject::BuildShaderTables()
{
	m_sbtHelper.AddMissProgram(c_missShaderName, {});
	D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = D3D12_GPU_DESCRIPTOR_HANDLE();// = m_srvUavHeap->GetGPUDescriptorHandleForHeapStart();
	auto heapPointer = reinterpret_cast<UINT64*>(srvUavHeapHandle.ptr);
	m_sbtHelper.AddRayGenerationProgram(c_raygenShaderName, { /*heapPointer*/ });
	m_sbtHelper.AddHitGroup(c_hitGroupName, {});
	uint32_t sbtSize = m_sbtHelper.ComputeSBTSize(D3D12RHI::GetDXCon(RHI::GetDefaultDevice())->GetDevice5());
	//	D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = m_srvUavHeap->GetGPUDescriptorHandleForHeapStart();
		// Create the SBT on the upload heap. This is required as the helper will use mapping to write the
		// SBT contents. After the SBT compilation it could be copied to the default heap for performance.
	m_sbtStorage = nv_helpers_dx12::CreateBuffer(D3D12RHI::GetDXCon(RHI::GetDefaultDevice())->GetDevice5(), sbtSize, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nv_helpers_dx12::kUploadHeapProps);
	StateObject->QueryInterface(IID_PPV_ARGS(&props));
	m_sbtHelper.Generate(m_sbtStorage, props);
}

void D3D12StateObject::CreateRaytracingOutputBuffer()
{
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	// The backbuffer is actually DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, but sRGB formats cannot be used
	// with UAVs. For accuracy we should convert to sRGB ourselves in the shader
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Alignment = 0;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resDesc.Width = 1000;
	resDesc.Height = 1000;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	ThrowIfFailed(D3D12RHI::GetDXCon(RHI::GetDefaultDevice())->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&m_outputResource)));

	UAVd = D3D12RHI::GetDXCon(RHI::GetDefaultDevice())->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	UAVd->CreateUnorderedAccessView(m_outputResource, nullptr, &UAVDesc, 0);
}

void D3D12StateObject::Trace(const RHIRayDispatchDesc& Desc, RHICommandList* T, D3D12FrameBuffer* target)
{
	if (TempCam != nullptr)
	{
		Data.IProj = glm::inverse(TempCam->GetProjection());
		Data.IView = glm::inverse(TempCam->GetView());
	}
	CBV->UpdateConstantBuffer(&Data, 0);
	D3D12CommandList* DXList = D3D12RHI::DXConv(T);
	DXList->GetCommandList()->SetComputeRootSignature(m_raytracingGlobalRootSignature);
	//DXList->GetCommandList()->SetComputeRootConstantBufferView(GlobalRootSignatureParams::CameraBuffer, )
	CBV->SetConstantBufferView(0, DXList->GetCMDList4(), GlobalRootSignatureParams::CameraBuffer, true, 0);
	DXList->GetCommandList()->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, ((D3D12RHIUAV*)target->GetUAV())->UAVDescriptor->GetGPUAddress());
	DXList->GetCommandList()->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, D3D12RHI::DXConv(High)->m_topLevelAccelerationStructure->GetGPUVirtualAddress());
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.RayGenerationShaderRecord.StartAddress = m_sbtStorage->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = m_sbtHelper.GetRayGenSectionSize();

	dispatchDesc.MissShaderTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + 64;
	dispatchDesc.MissShaderTable.SizeInBytes = m_sbtHelper.GetMissSectionSize();
	dispatchDesc.MissShaderTable.StrideInBytes = m_sbtHelper.GetMissEntrySize();

	dispatchDesc.HitGroupTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + 64 + 64;
	dispatchDesc.HitGroupTable.SizeInBytes = m_sbtHelper.GetHitGroupSectionSize();
	dispatchDesc.HitGroupTable.StrideInBytes = m_sbtHelper.GetHitGroupEntrySize();


	dispatchDesc.Width = Desc.Width;
	dispatchDesc.Height = Desc.Height;
	dispatchDesc.Depth = Desc.Depth;
	DXList->GetCMDList4()->SetPipelineState1(StateObject);
	DXList->GetCMDList4()->DispatchRays(&dispatchDesc);
}