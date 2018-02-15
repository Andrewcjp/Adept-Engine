#include "stdafx.h"
#include "D3D12Texture.h"
#include "D3D12RHI.h"
#include "../Core/Assets/ImageIO.h"
#include "../Core/Engine.h"
#include "../Core/Assets/ImageLoader.h"
#include <iostream>
#include "../Core/Performance/PerfManager.h"
#include "../Core/Utils/FileUtils.h"
float D3D12Texture::MipCreationTime = 0;
#include "../Core/Utils/StringUtil.h"
D3D12Texture::D3D12Texture() :D3D12Texture("house_diffuse.tga")
{
}

unsigned char * D3D12Texture::GenerateMip(int& startwidth, int& startheight, int bpp, unsigned char * StartData, int&mipsize, float ratio)
{
	std::string rpath = Engine::GetRootDir();
	rpath.append("\\asset\\DerivedDataCache\\");
	if (!FileUtils::exists_test3(rpath))
	{
		FileUtils::TryCreateDirectory(rpath);
	}
	rpath.append(TextureName);
	rpath.append("_mip_");

	int width = (int)(startwidth / ratio);
	int height = (int)(startheight / ratio);
	rpath.append(std::to_string(width));

	unsigned char *buffer = NULL;
	mipsize = (width*height*bpp);
	buffer = new unsigned char[mipsize];
	int stride = 4;
	int Sourcex = 0;
	int sourcey = 0;
	int nChannels = 0;
	float x_ratio = ((float)(startwidth - 1)) / width;
	float y_ratio = ((float)(startheight - 1)) / height;
	rpath.append(".bmp");
	if (FileUtils::exists_test3(rpath))
	{
		buffer = ImageLoader::instance->LoadSOILFile(&width, &height, &nChannels, rpath.c_str());
	}
	else
	{

		for (int x = 0; x < width*stride; x += stride)
		{
			for (int y = 0; y < height*stride; y += stride)
			{
				int y2 = (int)(y * ratio);
				int x2 = (int)(x * ratio);
				Sourcex = x2;
				sourcey = y2;
				glm::vec4 output;
#if 1
				glm::vec4 pixelA = glm::vec4(StartData[Sourcex + sourcey * startwidth], StartData[Sourcex + 1 + sourcey * startwidth], StartData[Sourcex + 2 + sourcey * startwidth], StartData[Sourcex + 3 + sourcey * startwidth]);
				glm::vec4 pixelB = glm::vec4(StartData[Sourcex + 4 + sourcey * startwidth], StartData[Sourcex + 4 + 1 + sourcey * startwidth], StartData[Sourcex + 4 + 2 + sourcey * startwidth], StartData[Sourcex + 4 + 3 + sourcey * startwidth]);
				int Targety = y2 + 1;
				glm::vec4 pixelC = glm::vec4(StartData[Sourcex + Targety * startwidth], StartData[Sourcex + 1 + Targety * startwidth], StartData[Sourcex + 2 + Targety * startwidth], StartData[Sourcex + 3 + Targety * startwidth]);
				glm::vec4 pixelD = glm::vec4(StartData[Sourcex + 4 + Targety * startwidth], StartData[Sourcex + 4 + 1 + Targety * startwidth], StartData[Sourcex + 4 + 2 + Targety * startwidth], StartData[Sourcex + 4 + 3 + Targety * startwidth]);
				float xdiff = (x_ratio*x) - (x_ratio*x);
				float ydiff = (y_ratio*y) - (y_ratio*y);
				output = (pixelA*(1 - xdiff)*(1 - ydiff)) + (pixelB * (xdiff)*(1 - ydiff)) +
					(pixelC*(ydiff)*(1 - xdiff)) + (pixelD * (xdiff*ydiff));
#else
				glm::vec4 nearn = glm::vec4(StartData[(y2 *startwidth) + x2], StartData[(y2 *startwidth) + 1 + x2], StartData[(y2 *startwidth) + 2 + x2], StartData[(y2 *startwidth) + 3 + x2]);
				output = nearn;
#endif

				buffer[x + (y*width)] = (unsigned char)output.r;
				buffer[x + 1 + (y*width)] = (unsigned char)output.g;
				buffer[x + 2 + (y*width)] = (unsigned char)output.b;
				buffer[x + 3 + (y*width)] = (unsigned char)output.a;
			}
		}
		if (startheight != 2048)
		{
			SOIL_save_image(rpath.c_str(), SOIL_SAVE_TYPE_BMP, width, height, 4, buffer);
		}
	}
	startheight = height;
	startwidth = width;
	return buffer;
}
struct Mipdata
{
	unsigned char* data;
	int size = 0;
};

unsigned char* D3D12Texture::GenerateMips(int count, int StartWidth, int StartHeight, unsigned char* startdata)
{
	long StartTime = PerfManager::get_nanos();
	int bpp = 4;
	int mipwidth = StartWidth;
	int mipheight = StartHeight;
	int mip0size = (StartWidth * StartHeight*bpp);
	int totalsize = mip0size;
	std::vector<Mipdata> Mips;
	unsigned char* output = startdata;
	for (int i = 0; i < count; i++)
	{
		if (mipwidth == 1 && mipheight == 1)
		{
			Miplevels = i;
			break;
		}
		int mipsize = 0;
		output = GenerateMip(mipwidth, mipheight, bpp, output, mipsize);

		totalsize += mipsize;
		Mipdata data;
		data.data = output;
		data.size = mipsize;
		Mips.push_back(data);
		Texturedatarray[i + 1].RowPitch = (mipwidth)* bpp;
		Texturedatarray[i + 1].SlicePitch = Texturedatarray[i + 1].RowPitch * (mipheight);
	}
	unsigned char*  finalbuffer = new unsigned char[totalsize];
	int Lastoffset = mip0size;
	memcpy(finalbuffer, startdata, mip0size);
	for (int i = 0; i < Mips.size(); i++)
	{
		memcpy((void*)(finalbuffer + Lastoffset), Mips[i].data, Mips[i].size);
		Texturedatarray[i + 1].pData = (finalbuffer + Lastoffset);
		Lastoffset += Mips[i].size;
	}
	long endtime = PerfManager::get_nanos();
	MipCreationTime += ((float)(endtime - StartTime ) / 1e6f);
	return finalbuffer;
}
D3D12Texture::D3D12Texture(std::string name)
{
	Miplevels = 9;
#if 1
	unsigned char *buffer = NULL;
	int width;
	int height;
	int bpp = 0;
	int nChannels;
	std::string rpath = Engine::GetRootDir();
	rpath.append("\\asset\\texture\\");
	rpath.append(name.c_str());
	TextureName = name;
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
		Miplevels = 1;
	}

	unsigned char*  finalbuffer = GenerateMips(Miplevels - 1, width, height, buffer);
	CreateTextureFromData(finalbuffer, 0, width, height, 4);
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
	ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
	D3D12RHI::Instance->BaseTextureHeap = m_srvHeap;
	m_srvHeap->SetName(L"Texture SRV");

	ID3D12Resource* textureUploadHeap;

	// Create the texture.
	{

		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = Miplevels;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_texture)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture, 0, Miplevels);

		// Create the GPU upload buffer.
		ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateCommittedResource(
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
		//append data!
		textureData.RowPitch = width * bits;
		textureData.SlicePitch = textureData.RowPitch * height;

		//array
		Texturedatarray[0] = textureData;
		UpdateSubresources(D3D12RHI::Instance->m_SetupCommandList, m_texture, textureUploadHeap, 0, 0, Miplevels, &Texturedatarray[0]);
		D3D12RHI::Instance->m_SetupCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = Miplevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		D3D12RHI::Instance->m_Primarydevice->CreateShaderResourceView(m_texture, &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	}
}
//_mipMapTextures is an array containing texture objects that need mipmaps to be generated. It needs a texture resource with mipmaps in D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE state.
//Textures are expected to be POT and in a format supporting unordered access, as well as the D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS set during creation.
//_device is the ID3D12Device
//GetNewCommandList() is supposed to return a new command list in recording state
//SubmitCommandList(commandList) is supposed to submit the command list to the command queue
//_mipMapComputeShader is an ID3DBlob of the compiled mipmap compute shader
