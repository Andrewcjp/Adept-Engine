#include "stdafx.h"
#include "ShaderMipMap.h"
#include "d3dx12.h"
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include <algorithm>
ShaderMipMap::ShaderMipMap()
{
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("MipmapCS", SHADER_COMPUTE);

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();
	D3D12Shader* shader = (D3D12Shader*)m_Shader;
	pCommandList = shader->CreateShaderCommandList();
}


ShaderMipMap::~ShaderMipMap()
{}
void ShaderMipMap::GenAllmips(int limit)
{
	int count = 0;
	for (int i = Targets.size()-1; i >= 0; i--)
	{
		if (count > limit)
		{
			break;
		}
		if (Targets[i] == nullptr)
		{
			Targets.erase(Targets.begin() + i);
			continue;
		}
		if (Targets[i]->Miplevels == Targets[i]->MipLevelsReadyNow)
		{
			Targets[i] = nullptr;
			Targets.erase(Targets.begin() + i);
			continue;
		}
		GenerateMipsForTexture(Targets[i], 1);
		//if complete		
		count++;
	}
}
void ShaderMipMap::GenerateMipsForTexture(D3D12Texture* tex, int maxcount)
{
	int requiredHeapSize = tex->Miplevels;
	D3D12Shader* shader = (D3D12Shader*)m_Shader;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 2 * requiredHeapSize;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ID3D12DescriptorHeap *descriptorHeap;
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap)));
	UINT descriptorSize = D3D12RHI::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srcTextureSRVDesc = {};
	srcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	//Prepare the unordered access view description for the destination texture
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	shader->ResetList(pCommandList);


	pCommandList->SetPipelineState(shader->GetPipelineShader()->m_pipelineState);
	pCommandList->SetComputeRootSignature(shader->GetPipelineShader()->m_rootSignature);
	pCommandList->SetDescriptorHeaps(1, &descriptorHeap);



	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
	CD3DX12_CPU_DESCRIPTOR_HANDLE currentCPUHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE currentGPUHandle(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorSize);
	int CurrentTopMip = tex->MipLevelsReadyNow;
	int target = std::min((CurrentTopMip + maxcount), (tex->Miplevels - 1));
	for (uint32_t TopMip = (CurrentTopMip - 1); TopMip < target; TopMip++)
	{
		CurrentTopMip = TopMip + 1;
		uint32_t dstWidth = std::max(tex->width >> (TopMip + 1), 1);
		uint32_t dstHeight = std::max(tex->height >> (TopMip + 1), 1);

		//Create shader resource view for the source texture in the descriptor heap
		srcTextureSRVDesc.Format = tex->GetResource()->GetDesc().Format;
		srcTextureSRVDesc.Texture2D.MipLevels = 1;
		srcTextureSRVDesc.Texture2D.MostDetailedMip = TopMip;
		//tex->
		D3D12RHI::GetDevice()->CreateShaderResourceView(tex->GetResource(), &srcTextureSRVDesc, currentCPUHandle);
		currentCPUHandle.Offset(1, descriptorSize);

		//Create unordered access view for the destination texture in the descriptor heap
		destTextureUAVDesc.Format = tex->GetResource()->GetDesc().Format;
		destTextureUAVDesc.Texture2D.MipSlice = TopMip + 1;
		ID3D12Resource* UAV = nullptr;
		D3D12RHI::GetDevice()->CreateUnorderedAccessView(tex->GetResource(), UAV, &destTextureUAVDesc, currentCPUHandle);
		currentCPUHandle.Offset(1, descriptorSize);

		//Pass the destination texture pixel size to the shader as constants
		float over = (1.0f / dstWidth);
		struct DWParam
		{
			DWParam(FLOAT f) : Float(f) {}
			DWParam(UINT u) : Uint(u) {}

			void operator= (FLOAT f) { Float = f; }
			void operator= (UINT u) { Uint = u; }

			union
			{
				FLOAT Float;
				UINT Uint;
			};
		};

		pCommandList->SetComputeRoot32BitConstant(0, DWParam(1.0f / dstWidth).Uint, 0);
		pCommandList->SetComputeRoot32BitConstant(0, DWParam(1.0f / dstHeight).Uint, 1);

		//Pass the source and destination texture views to the shader via descriptor tables
		pCommandList->SetComputeRootDescriptorTable(1, currentGPUHandle);
		currentGPUHandle.Offset(1, descriptorSize);
		pCommandList->SetComputeRootDescriptorTable(2, currentGPUHandle);
		currentGPUHandle.Offset(1, descriptorSize);

		//Dispatch the compute shader with one thread per 8x8 pixels
		pCommandList->Dispatch(std::max(dstWidth / 8, 1u), std::max(dstHeight / 8, 1u), 1);
		//ensure Completion
		pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(UAV));
	}
	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	pCommandList->Close();

	D3D12RHI::Instance->ExecList(pCommandList);
	tex->MipLevelsReadyNow = CurrentTopMip + 1;
	tex->UpdateSRV();
}

std::vector<Shader::ShaderParameter> ShaderMipMap::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;

	return Output;
}
