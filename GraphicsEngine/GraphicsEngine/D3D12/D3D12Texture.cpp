#include "stdafx.h"
#include "D3D12Texture.h"
#include "D3D12RHI.h"
#include "../Core/Assets/ImageIO.h"
#include "../Core/Engine.h"
#include "../Core/Assets/ImageLoader.h"
D3D12Texture::D3D12Texture() :D3D12Texture("house_diffuse.tga")
{

}
D3D12Texture::D3D12Texture(std::string name)
{
#if 1
	unsigned char *buffer = NULL;
	int width;
	int height;
	int bpp = 0;
	int nChannels;
	std::string rpath = Engine::GetRootDir();
	rpath.append("\\asset\\texture\\");
	rpath.append(name.c_str());
	//rpath.append("house_diffuse.tga");
	if (rpath.find(".tga") == -1)
	{
		buffer = ImageLoader::instance->LoadSOILFile(&width, &height, &nChannels, rpath.c_str());
	}
	else
	{
		if (ImageIO::LoadTGA(rpath.c_str(), &buffer, &width, &height, &bpp, &nChannels) != E_IMAGEIO_SUCCESS)
		{
			return;
		}
	}

	CreateTextureFromData(buffer, 0, width, height, 4);
#else
	CreateTextureFromData(GenerateCheckerBoardTextureData(), 0, TextureWidth, TextureHeight, TexturePixelSize);
#endif
}


D3D12Texture::~D3D12Texture()
{
}
void D3D12Texture::CreateTexture()
{


}
UINT8* D3D12Texture::GenerateCheckerBoardTextureData()
{
	const UINT rowPitch = TextureWidth * TexturePixelSize;
	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
	const UINT cellHeight = TextureWidth >> 3;	// The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * TextureHeight;

	//std::vector<UINT8> data(textureSize);
	UINT8* data = new UINT8[textureSize];
	UINT8* pData = &data[0];

	for (UINT n = 0; n < textureSize; n += TexturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;		// R
			pData[n + 1] = 0x00;	// G
			pData[n + 2] = 0x00;	// B
			pData[n + 3] = 0xff;	// A
		}
		else
		{
			pData[n] = 0xff;		// R
			pData[n + 1] = 0xff;	// G
			pData[n + 2] = 0xff;	// B
			pData[n + 3] = 0xff;	// A
		}
	}

	return data;
}

void D3D12Texture::Bind(int unit)
{

}
void D3D12Texture::Bind(CommandListDef* list)
{
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	list->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void D3D12Texture::FreeTexture()
{
}

void D3D12Texture::SetTextureID(int id)
{
}

void D3D12Texture::CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha)
{
}

void D3D12Texture::CreateTextureFromData(void * data, int type, int width, int height, int bits)
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(D3D12RHI::Instance->m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));


	ID3D12Resource* textureUploadHeap;

	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ThrowIfFailed(D3D12RHI::Instance->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_texture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture, 0, 1);

		// Create the GPU upload buffer.
		ThrowIfFailed(D3D12RHI::Instance->m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap)));

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the Texture2D.
		//std::vector<UINT8> texture = GenerateTextureData();

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = data;
		textureData.RowPitch = width * bits;
		textureData.SlicePitch = textureData.RowPitch * height;

		UpdateSubresources(D3D12RHI::Instance->m_SetupCommandList, m_texture, textureUploadHeap, 0, 0, 1, &textureData);
		D3D12RHI::Instance->m_SetupCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		D3D12RHI::Instance->m_device->CreateShaderResourceView(m_texture, &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	}
}
