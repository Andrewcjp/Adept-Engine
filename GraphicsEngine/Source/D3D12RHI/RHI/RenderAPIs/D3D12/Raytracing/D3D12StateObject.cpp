#include "D3D12StateObject.h"
#include "D3D12HighLevelAccelerationStructure.h"
#include "Rendering/RayTracing/Shader_RTBase.h"
#include "Rendering/RayTracing/ShaderBindingTable.h"
#include "RHI/RenderAPIs/D3D12/D3D12Buffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "RHI/RenderAPIs/D3D12/D3D12DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/DescriptorHeapManager.h"
#include "RHI/RenderAPIs/D3D12/ThirdParty/DXRHelper.h"
#include "Rendering/Shaders/Raytracing/Reflections/Shader_ReflectionRaygen.h"
#include "../GPUResource.h"
#include "../DXDescriptor.h"
#include "../DXMemoryManager.h"
#if WIN10_1809
D3D12StateObject::D3D12StateObject(DeviceContext* D, RHIStateObjectDesc desc) :RHIStateObject(D, desc)
{
	RayDataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant, D);
	RayDataBuffer->CreateConstantBuffer(sizeof(RayArgs), MaxRayDispatchPerFrame);

}


D3D12StateObject::~D3D12StateObject()
{}

void D3D12StateObject::Build()
{
	ShaderTable->Validate();
	CreateRootSignatures();
	CreateStateObject();
	BuildShaderTables();
	Log::LogMessage("DXR State Object built");
}



void D3D12StateObject::AddShaders(CD3DX12_STATE_OBJECT_DESC & Pipe)
{
	for (int i = 0; i < ShaderTable->HitGroups.size(); i++)
	{
		AddShaderLibrary(Pipe, ShaderTable->HitGroups[i]->HitShader);
		AddShaderLibrary(Pipe, ShaderTable->HitGroups[i]->AnyHitShader);
		//DXR:: check this is okay 
		CreateLocalRootSigShaders(Pipe, ShaderTable->HitGroups[i]->HitShader);
		CreateLocalRootSigShaders(Pipe, ShaderTable->HitGroups[i]->AnyHitShader);
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
	CD3DX12_STATE_OBJECT_DESC RTPipe;
	RTPipe.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
	AddShaders(RTPipe);
	AddHitGroups(RTPipe);

	auto shaderConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	shaderConfig->Config(Desc.PayloadSize, Desc.AttibuteSize);

	auto globalRootSignature = RTPipe.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature);

	auto pipelineConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();


	pipelineConfig->Config(Desc.MaxRecursionDepth);

	D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetDevice5()->CreateStateObject(RTPipe, IID_PPV_ARGS(&StateObject));
}

void D3D12StateObject::AddHitGroups(CD3DX12_STATE_OBJECT_DESC &RTPipe)
{
	for (int i = 0; i < ShaderTable->HitGroups.size(); i++)
	{
		auto hitGroup = RTPipe.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		std::wstring HitName = StringUtils::ConvertStringToWide(ShaderTable->HitGroups[i]->HitShader->GetExports()[0]);
		hitGroup->SetClosestHitShaderImport(HitName.c_str());
		if (ShaderTable->HitGroups[i]->AnyHitShader != nullptr)
		{
			std::wstring anyHitName = StringUtils::ConvertStringToWide(ShaderTable->HitGroups[i]->AnyHitShader->GetExports()[0]);
			hitGroup->SetAnyHitShaderImport(anyHitName.c_str());
		}
		hitGroup->SetHitGroupExport(ShaderTable->HitGroups[i]->WName.c_str());
		//DXR: todo GEO type
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}
}

void D3D12StateObject::AddShaderLibrary(CD3DX12_STATE_OBJECT_DESC &RTPipe, Shader_RTBase* Shader)
{
	if (Shader == nullptr)
	{
		return;
	}
	auto lib = RTPipe.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	ShaderBlob* B = D3D12RHI::DXConv(Shader->GetShaderProgram())->GetShaderBlobs()->RTLibBlob;
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
	if (shader == nullptr || shader->GetShaderParameters().size() == 0)
	{
		return;
	}
	if (shader->GetStage() == ERTShaderType::RayGen)
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
		std::vector<void*> GPUPtrs;
		WriteBinds(ShaderTable->HitGroups[i]->HitShader, GPUPtrs);
		m_sbtHelper.AddHitGroup(ShaderTable->HitGroups[i]->WName, GPUPtrs);
	}

	for (int i = 0; i < ShaderTable->RayGenShaders.size(); i++)
	{
		std::vector<void*> GPUPtrs;
		WriteBinds(ShaderTable->RayGenShaders[i], GPUPtrs);
		m_sbtHelper.AddRayGenerationProgram(ShaderTable->RayGenShaders[i]->GetFirstExportWide(), GPUPtrs);
	}
	for (int i = 0; i < ShaderTable->MissShaders.size(); i++)
	{
		std::vector<void*> GPUPtrs;
		WriteBinds(ShaderTable->MissShaders[i], GPUPtrs);
		m_sbtHelper.AddMissProgram(ShaderTable->MissShaders[i]->GetFirstExportWide(), GPUPtrs);
	}
	uint32_t sbtSize = m_sbtHelper.ComputeSBTSize(D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetDevice5()) * 4;
	//	D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = m_srvUavHeap->GetGPUDescriptorHandleForHeapStart();
		// Create the SBT on the upload heap. This is required as the helper will use mapping to write the
		// SBT contents. After the SBT compilation it could be copied to the default heap for performance.
	if (m_sbtStorage == nullptr || CurrentSBTSize < sbtSize)
	{
		EnqueueSafeRHIRelease(SBTData);
		CurrentSBTSize = sbtSize;
		AllocDesc desc;
		desc.Size = sbtSize;
		desc.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
		desc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.Size, D3D12_RESOURCE_FLAG_NONE);
		D3D12RHI::DXConv(Device)->GetMemoryManager()->AllocUploadTemporary(desc, &SBTData);
		m_sbtStorage = SBTData->GetResource();
	}
	StateObject->QueryInterface(IID_PPV_ARGS(&props));
	m_sbtHelper.Generate(m_sbtStorage, props);
}

void D3D12StateObject::WriteBinds(Shader_RTBase* shader, std::vector<void *> &Pointers)
{
	for (int i = 0; i < shader->LocalRootSig.GetNumBinds(); i++)
	{
		const RSBind* bind = shader->LocalRootSig.GetBind(i);
		if (bind->BindType == ERSBindType::Texture)
		{
			D3D12Texture* DTex = D3D12RHI::DXConv(bind->Texture.Get());
			if (DTex != nullptr)
			{
				auto heapPointer = reinterpret_cast<uint64_t*>(DTex->GetDescriptor(bind->View)->GetGPUAddress().ptr);
				Pointers.push_back(heapPointer);
			}
		}
		else if (bind->BindType == ERSBindType::BufferSRV)
		{
			D3D12Buffer* DTex = D3D12RHI::DXConv(bind->BufferTarget);
			DXDescriptor* d = DTex->GetDescriptor(bind->View);
			UINT64 Ptr = d->GetGPUAddress().ptr;
			Pointers.push_back((void*)Ptr);
		}
		else if (bind->BindType == ERSBindType::FrameBuffer)
		{
			D3D12FrameBuffer* DTex = D3D12RHI::DXConv(bind->Framebuffer);
			auto heapPointer = reinterpret_cast<uint64_t*>(DTex->GetDescriptor(bind->View)->GetGPUAddress().ptr);
			Pointers.push_back(heapPointer);
		}
	}
}

void D3D12StateObject::RebuildShaderTable()
{
	BuildShaderTables();
}

void D3D12StateObject::BindToList(D3D12CommandList * List)
{
	List->GetCommandList()->SetComputeRootSignature(m_raytracingGlobalRootSignature);
	List->GetRootSig()->SetRootSig(ShaderTable->GlobalRootSig.Params);
}

void D3D12StateObject::Trace(const RHIRayDispatchDesc& DispatchDesc, RHICommandList* T, D3D12FrameBuffer* target)
{
	D3D12CommandList* DXList = D3D12RHI::DXConv(T);
	if (DispatchDesc.PushRayArgs)
	{
		DXList->SetRootConstant(8, 2, ((void*)&DispatchDesc.RayArguments), 0);
	}
	DXList->GetCommandList()->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, D3D12RHI::DXConv(target)->GetDescriptor(RHIViewDesc::DefaultUAV())->GetGPUAddress());
	DXList->GetCommandList()->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, D3D12RHI::DXConv(HighLevelStructure)->m_topLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress());

	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	dispatchDesc.RayGenerationShaderRecord.StartAddress = m_sbtStorage->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = m_sbtHelper.GetRayGenSectionSize();

	dispatchDesc.MissShaderTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + m_sbtHelper.GetRayGenSectionSize();
	dispatchDesc.MissShaderTable.SizeInBytes = m_sbtHelper.GetMissSectionSize();
	dispatchDesc.MissShaderTable.StrideInBytes = m_sbtHelper.GetMissEntrySize();

	dispatchDesc.HitGroupTable.StartAddress = m_sbtStorage->GetGPUVirtualAddress() + m_sbtHelper.GetRayGenSectionSize() + m_sbtHelper.GetMissSectionSize();
	dispatchDesc.HitGroupTable.SizeInBytes = m_sbtHelper.GetHitGroupSectionSize();
	dispatchDesc.HitGroupTable.StrideInBytes = m_sbtHelper.GetHitGroupEntrySize();

	dispatchDesc.Width = DispatchDesc.Width;
	dispatchDesc.Height = DispatchDesc.Height;
	dispatchDesc.Depth = DispatchDesc.Depth;

	DXList->GetCMDList4()->SetPipelineState1(StateObject);
	DXList->GetCMDList4()->DispatchRays(&dispatchDesc);
}
#endif