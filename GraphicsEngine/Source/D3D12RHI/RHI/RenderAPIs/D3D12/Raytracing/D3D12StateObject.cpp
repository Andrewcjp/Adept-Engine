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
#include "RHI/RHIRootSigniture.h"
#include "../D3D12RHITexture.h"
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
	//	Desc.ShaderRecords.Validate();
	CreateRootSignatures();
	CreateStateObject();
	//BuildShaderTables();
	Log::LogMessage("DXR State Object built");
}



void D3D12StateObject::AddShaders(CD3DX12_STATE_OBJECT_DESC & Pipe)
{
	for (int i = 0; i < Desc.ShaderRecords.HitGroups.size(); i++)
	{
		AddShaderLibrary(Pipe, Desc.ShaderRecords.HitGroups[i]->HitShader);
		AddShaderLibrary(Pipe, Desc.ShaderRecords.HitGroups[i]->AnyHitShader);
		AddShaderLibrary(Pipe, Desc.ShaderRecords.HitGroups[i]->IntersectionShader);		
		//DXR:: check this is okay 
		CreateLocalRootSigShaders(Pipe, Desc.ShaderRecords.HitGroups[i]->HitShader, Desc.ShaderRecords.HitGroups[i]);
		//CreateLocalRootSigShaders(Pipe, Desc.ShaderRecords.HitGroups[i]->AnyHitShader, Desc.ShaderRecords.HitGroups[i]);
	}

	for (int i = 0; i < Desc.ShaderRecords.MissShaders.size(); i++)
	{
		AddShaderLibrary(Pipe, Desc.ShaderRecords.MissShaders[i]);
		CreateLocalRootSigShaders(Pipe, Desc.ShaderRecords.MissShaders[i]);
	}
	for (int i = 0; i < Desc.ShaderRecords.RayGenShaders.size(); i++)
	{
		AddShaderLibrary(Pipe, Desc.ShaderRecords.RayGenShaders[i]);
		CreateLocalRootSigShaders(Pipe, Desc.ShaderRecords.RayGenShaders[i]);
	}
}

void D3D12StateObject::CreateStateObject()
{
	CD3DX12_STATE_OBJECT_DESC RTPipe;
	RTPipe.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);
	AddShaders(RTPipe);
	AddHitGroups(RTPipe);

	auto globalRootSignature = RTPipe.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature);

	auto shaderConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	shaderConfig->Config(Desc.PayloadSize, Desc.AttibuteSize);

	auto pipelineConfig = RTPipe.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	pipelineConfig->Config(Desc.MaxRecursionDepth);

	D3D12RHI::DXConv(RHI::GetDefaultDevice())->GetDevice6()->CreateStateObject(RTPipe, ID_PASS(&StateObject));
}

void D3D12StateObject::AddHitGroups(CD3DX12_STATE_OBJECT_DESC &RTPipe)
{
	for (int i = 0; i < Desc.ShaderRecords.HitGroups.size(); i++)
	{
		auto hitGroup = RTPipe.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		std::wstring HitName = StringUtils::ConvertStringToWide(Desc.ShaderRecords.HitGroups[i]->HitShader->GetExports()[0]);
		hitGroup->SetClosestHitShaderImport(HitName.c_str());
		if (Desc.ShaderRecords.HitGroups[i]->AnyHitShader != nullptr)
		{
			std::wstring anyHitName = StringUtils::ConvertStringToWide(Desc.ShaderRecords.HitGroups[i]->AnyHitShader->GetExports()[0]);
			hitGroup->SetAnyHitShaderImport(anyHitName.c_str());
		}
		if (Desc.ShaderRecords.HitGroups[i]->IntersectionShader != nullptr)
		{
			std::wstring intersectionexport = StringUtils::ConvertStringToWide(Desc.ShaderRecords.HitGroups[i]->IntersectionShader->GetExports()[0]);
			hitGroup->SetIntersectionShaderImport(intersectionexport.c_str());
		}
		hitGroup->SetHitGroupExport(Desc.ShaderRecords.HitGroups[i]->WName.c_str());
		hitGroup->SetHitGroupType((D3D12_HIT_GROUP_TYPE)Desc.ShaderRecords.HitGroups[i]->GroupType);
	}
}

void D3D12StateObject::AddShaderLibrary(CD3DX12_STATE_OBJECT_DESC &RTPipe, Shader_RTBase* Shader)
{
	if (Shader == nullptr)
	{
		return;
	}
	if (Exports.find(Shader->GetExports()[0]) != Exports.end())
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
		Exports.emplace(Shader->GetExports()[i]);
	}
}

void D3D12StateObject::CreateRootSignatures()
{
	RootSignitureCreateInfo Info;
	Info.IsGlobalSig = true;
	D3D12Shader::CreateRootSig(&m_raytracingGlobalRootSignature, Desc.ShaderRecords.GlobalRS.Params, Device, true, RHISamplerDesc::GetDefault(), Info);
}

void D3D12StateObject::CreateLocalRootSigShaders(CD3DX12_STATE_OBJECT_DESC & raytracingPipeline, Shader_RTBase* shader, ShaderHitGroup* Grp)
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
	//if (shader->GetStage() == ERTShaderType::RayGen)
	//{
	//	std::vector<ShaderParameter> Hask;
	//	Hask.push_back(ShaderParameter(ShaderParamType::CBV, 5, 66));
	//	D3D12Shader::CreateRootSig(&m_raytracingLocalRootSignature, Hask, Device, true, std::vector<RHISamplerDesc>(), Info);
	//}
	//else
	{
		D3D12Shader::CreateRootSig(&m_raytracingLocalRootSignature, shader->GetShaderParameters(), Device, true, std::vector<RHISamplerDesc>(), Info);
	}
	// Local root signature to be used in a hit group.
	auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	localRootSignature->SetRootSignature(m_raytracingLocalRootSignature);
	// Define explicit shader association for the local root signature. 
	{
		auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		if (Grp != nullptr)
		{
			std::wstring ExportName = (Grp->WName);
			rootSignatureAssociation->AddExport(ExportName.c_str());
		}
		else
		{
			std::wstring ExportName = StringUtils::ConvertStringToWide(shader->GetExports()[0]);
			rootSignatureAssociation->AddExport(ExportName.c_str());
		}
	}
}

void D3D12StateObject::BuildShaderTables()
{
	//DXR: Push default values to table.
	m_sbtHelper.Reset();
	for (int i = 0; i < ShaderTable->HitGroupInstances.size(); i++)
	{
		std::vector<void*> GPUPtrs;
		WriteBinds2(ShaderTable->HitGroupInstances[i]->mClosetHitRS, GPUPtrs);
		m_sbtHelper.AddHitGroup(ShaderTable->HitGroupInstances[i]->WName, GPUPtrs);
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
	StateObject->QueryInterface(ID_PASS(&props));
	m_sbtHelper.Generate(m_sbtStorage, props);
}

void D3D12StateObject::WriteBinds2(RHIRootSigniture & sig, std::vector<void *> &Pointers)
{
	for (int i = 0; i < sig.GetNumBinds(); i++)
	{
		const RSBind* bind = sig.GetBind(i);
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
		else if (bind->BindType == ERSBindType::Texture2)
		{
			D3D12RHITexture* DTex = D3D12RHI::DXConv(bind->Texture2);
			if (DTex != nullptr)
			{
				auto heapPointer = reinterpret_cast<uint64_t*>(DTex->GetDescriptor(bind->View, nullptr)->GetGPUAddress().ptr);
				Pointers.push_back(heapPointer);
			}
			else
			{
				//null descriptors!
				Pointers.push_back(0);
			}
		}
	}
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
		else if (bind->BindType == ERSBindType::Texture2)
		{
			D3D12RHITexture* DTex = D3D12RHI::DXConv(bind->Texture2);
			if (DTex != nullptr)
			{
				auto heapPointer = reinterpret_cast<uint64_t*>(DTex->GetDescriptor(bind->View, nullptr)->GetGPUAddress().ptr);
				Pointers.push_back(heapPointer);
			}
			else
			{
				//null descriptors!
				Pointers.push_back(0);
			}
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
	List->GetRootSig()->SetRootSig(Desc.ShaderRecords.GlobalRS.Params);
}

void D3D12StateObject::Trace(const RHIRayDispatchDesc& DispatchDesc, RHICommandList* T, D3D12FrameBuffer* target)
{
	D3D12CommandList* DXList = D3D12RHI::DXConv(T);

	DXList->GetRootSig()->SetRootSig(Desc.ShaderRecords.GlobalRS.Params);
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