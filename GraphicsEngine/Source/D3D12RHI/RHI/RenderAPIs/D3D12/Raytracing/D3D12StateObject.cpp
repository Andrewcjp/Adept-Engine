#include "D3D12StateObject.h"
#include "D3D12HighLevelAccelerationStructure.h"
#include "Rendering/RayTracing/Shader_RTBase.h"
#include "Rendering/RayTracing/ShaderBindingTable.h"
#include "RHI/RenderAPIs/D3D12/D3D12Buffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "RHI/RenderAPIs/D3D12/D3D12DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/DescriptorGroup.h"
#include "RHI/RenderAPIs/D3D12/DescriptorHeapManager.h"
#include "RHI/RenderAPIs/D3D12/ThirdParty/DXRHelper.h"

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

void D3D12StateObject::AddShaders(CD3DX12_STATE_OBJECT_DESC & Pipe)
{
	for (int i = 0; i < ShaderTable->HitGroups.size(); i++)
	{
		AddShaderLibrary(Pipe, ShaderTable->HitGroups[i]->HitShader);
		//DXR:: check this is okay 
		CreateLocalRootSigShaders(Pipe, ShaderTable->HitGroups[i]->HitShader);
	}
	for (int i = 0; i < ShaderTable->RayGenShaders.size(); i++)
	{
		AddShaderLibrary(Pipe, ShaderTable->RayGenShaders[i]);
		CreateLocalRootSigShaders(Pipe, ShaderTable->RayGenShaders[i]);
	}
	for (int i = 0; i < ShaderTable->MissShaders.size(); i++)
	{
		AddShaderLibrary(Pipe, ShaderTable->MissShaders[i]);
		CreateLocalRootSigShaders(Pipe, ShaderTable->MissShaders[i]);
	}
}

void D3D12StateObject::CreateStateObject()
{
	//CD3D12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
	CD3DX12_STATE_OBJECT_DESC RTPipe;
	RTPipe.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
	AddShaders(RTPipe);
	AddHitGroups(RTPipe);

	auto shaderConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = sizeof(glm::vec4);    // float4 pixelColor
	UINT attributeSize = sizeof(glm::vec2);  // float2 barycentrics
	shaderConfig->Config(payloadSize, attributeSize);



	auto globalRootSignature = RTPipe.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature);

	auto pipelineConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	// PERFOMANCE TIP: Set max recursion depth as low as needed 
	// as drivers may apply optimization strategies for low recursion depths.
	UINT maxRecursionDepth = 1; // ~ primary rays only. 
	pipelineConfig->Config(maxRecursionDepth);

	D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetDevice5()->CreateStateObject(RTPipe, IID_PPV_ARGS(&StateObject));
}

void D3D12StateObject::AddHitGroups(CD3DX12_STATE_OBJECT_DESC &RTPipe)
{
	for (int i = 0; i < ShaderTable->HitGroups.size(); i++)
	{
		auto hitGroup = RTPipe.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		std::wstring HitName = StringUtils::ConvertStringToWide(ShaderTable->HitGroups[i]->HitShader->GetExports()[0]);
		hitGroup->SetClosestHitShaderImport(HitName.c_str());
		std::wstring Groupname = StringUtils::ConvertStringToWide(ShaderTable->HitGroups[i]->Name);
		hitGroup->SetHitGroupExport(Groupname.c_str());
		//DXR: todo GEO type
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}
}

void D3D12StateObject::AddShaderLibrary(CD3DX12_STATE_OBJECT_DESC &RTPipe, Shader_RTBase* Shader)
{
	auto lib = RTPipe.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	IDxcBlob* B = ((D3D12Shader*)Shader->GetShaderProgram())->GetShaderBlobs()->RTLibBlob;
	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(B->GetBufferPointer(), B->GetBufferSize());
	lib->SetDXILLibrary(&libdxil);
	for (int i = 0; i < Shader->GetExports().size(); i++)
	{
		std::wstring Conv = StringUtils::ConvertStringToWide(Shader->GetExports()[i]);
		lib->DefineExport(Conv.c_str());
	}

}

void D3D12StateObject::CreateRootSignatures()
{
	D3D12Shader::CreateRootSig(&m_raytracingGlobalRootSignature, ShaderTable->GlobalRootSig.Params, Device, true, RHISamplerDesc::GetDefault());
}

void D3D12StateObject::CreateLocalRootSigShaders(CD3DX12_STATE_OBJECT_DESC & raytracingPipeline, Shader_RTBase* shader)
{
	if (shader->GetShaderParameters().size() == 0)
	{
		return;
	}
	// Ray gen and miss shaders in this sample are not using a local root signature and thus one is not associated with them.
	ID3D12RootSignature* m_raytracingLocalRootSignature = nullptr;
	RootSignitureCreateInfo Info;
	Info.IsLocalSig = true;
	D3D12Shader::CreateRootSig(&m_raytracingLocalRootSignature, shader->GetShaderParameters(), Device, true, std::vector<RHISamplerDesc>(), Info);

	// Local root signature to be used in a hit group.
	auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	localRootSignature->SetRootSignature(m_raytracingLocalRootSignature);
	// Define explicit shader association for the local root signature. 
	{
		auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);

		std::wstring ExportName = StringUtils::ConvertStringToWide(shader->GetExports()[0]);
		rootSignatureAssociation->AddExport(ExportName.c_str());
	}
}

void D3D12StateObject::BuildShaderTables()
{
	//DXR: Push default values to table.
	m_sbtHelper.Reset();
	for (int i = 0; i < ShaderTable->HitGroups.size(); i++)
	{
		std::wstring Name = StringUtils::ConvertStringToWide(ShaderTable->HitGroups[i]->Name);
		std::vector<void*> GPUPtrs;
		WriteBinds(ShaderTable->HitGroups[i]->HitShader, GPUPtrs);
		m_sbtHelper.AddHitGroup(Name, GPUPtrs);
	}

	for (int i = 0; i < ShaderTable->RayGenShaders.size(); i++)
	{
		std::wstring Name = StringUtils::ConvertStringToWide(ShaderTable->RayGenShaders[i]->GetExports()[0]);
		std::vector<void*> GPUPtrs;
		WriteBinds(ShaderTable->RayGenShaders[i], GPUPtrs);
		m_sbtHelper.AddRayGenerationProgram(Name, GPUPtrs);
	}
	for (int i = 0; i < ShaderTable->MissShaders.size(); i++)
	{
		std::wstring Name = StringUtils::ConvertStringToWide(ShaderTable->MissShaders[i]->GetExports()[0]);

		std::vector<void*> GPUPtrs;
		WriteBinds(ShaderTable->MissShaders[i], GPUPtrs);
		m_sbtHelper.AddMissProgram(Name, GPUPtrs);
	}

	uint32_t sbtSize = m_sbtHelper.ComputeSBTSize(D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetDevice5());
	//	D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = m_srvUavHeap->GetGPUDescriptorHandleForHeapStart();
		// Create the SBT on the upload heap. This is required as the helper will use mapping to write the
		// SBT contents. After the SBT compilation it could be copied to the default heap for performance.
	if (m_sbtStorage == nullptr || CurrentSBTSize < sbtSize)
	{
		CurrentSBTSize = sbtSize;
		m_sbtStorage = nv_helpers_dx12::CreateBuffer(D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetDevice5(), sbtSize, D3D12_RESOURCE_FLAG_NONE,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nv_helpers_dx12::kUploadHeapProps);
	}
	StateObject->QueryInterface(IID_PPV_ARGS(&props));
	m_sbtHelper.Generate(m_sbtStorage, props);
}

void D3D12StateObject::WriteBinds(Shader_RTBase* shader, std::vector<void *> &Data)
{
	for (int i = 0; i < shader->Buffers.size(); i++)
	{
		D3D12Buffer* DTex = D3D12RHI::DXConv(shader->Buffers[i]);
		DTex->SetupBufferSRV();
		auto heapPointer = reinterpret_cast<uint64_t*>(DTex->GetDescriptor()->GetGPUAddress().ptr);
		Data.push_back(heapPointer);
	}
	for (int t = 0; t < shader->Textures.size(); t++)
	{
		D3D12Texture* DTex = D3D12RHI::DXConv(shader->Textures[t].Get());
		auto heapPointer = reinterpret_cast<uint64_t*>(DTex->GetDescriptor()->GetGPUAddress().ptr);
		Data.push_back(heapPointer);
	}

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
	resDesc.Width = 12;
	resDesc.Height = 12;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	ThrowIfFailed(D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&m_outputResource)));
	return;
	UAVd = D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	UAVd->CreateUnorderedAccessView(m_outputResource, nullptr, &UAVDesc, 0);
}

void D3D12StateObject::RebuildShaderTable()
{
	//CreateStateObject();
	BuildShaderTables();
}

void D3D12StateObject::BindToList(D3D12CommandList * List)
{
	List->GetCommandList()->SetComputeRootSignature(m_raytracingGlobalRootSignature);
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


	//DXR: Todo: move to above RHI
	CBV->SetConstantBufferView(0, DXList->GetCMDList4(), GlobalRootSignatureParams::CameraBuffer, true, 0);
	DXList->GetCommandList()->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, ((D3D12RHIUAV*)target->GetUAV())->UAVDescriptor->GetGPUAddress());
	DXList->GetCommandList()->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, D3D12RHI::DXConv(High)->m_topLevelAccelerationStructure->GetGPUVirtualAddress());

	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.RayGenerationShaderRecord.StartAddress = m_sbtStorage->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = m_sbtHelper.GetRayGenSectionSize();

	dispatchDesc.MissShaderTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + m_sbtHelper.GetRayGenSectionSize();
	dispatchDesc.MissShaderTable.SizeInBytes = m_sbtHelper.GetMissSectionSize();
	dispatchDesc.MissShaderTable.StrideInBytes = m_sbtHelper.GetMissEntrySize();

	dispatchDesc.HitGroupTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + m_sbtHelper.GetRayGenSectionSize() + m_sbtHelper.GetMissSectionSize();
	dispatchDesc.HitGroupTable.SizeInBytes = m_sbtHelper.GetHitGroupSectionSize();
	dispatchDesc.HitGroupTable.StrideInBytes = m_sbtHelper.GetHitGroupEntrySize();


	dispatchDesc.Width = Desc.Width;
	dispatchDesc.Height = Desc.Height;
	dispatchDesc.Depth = Desc.Depth;

	DXList->GetCMDList4()->SetPipelineState1(StateObject);
	DXList->GetCMDList4()->DispatchRays(&dispatchDesc);
}