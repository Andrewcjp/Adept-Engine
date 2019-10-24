
#include <iostream>
#include "Core/Assets/ImageIO.h"
#include "Core/Engine.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/FileUtils.h"
#include "GPUResource.h"
#include "ThirdParty/NVDDS/DDSTextureLoader12.h"
#include "DescriptorHeap.h"
#include "D3D12Texture.h"
#include "D3D12DeviceContext.h"
#include "D3D12CommandList.h"
#include "DescriptorHeapManager.h"
#include "DescriptorGroup.h"
#include "DXMemoryManager.h"
#include "DXDescriptor.h"

CreateChecker(D3D12Texture);
#define USE_CPUFALLBACK_TOGENMIPS_ATRUNTIME 0
float D3D12Texture::MipCreationTime = 0;
D3D12Texture::D3D12Texture(DeviceContext* inDevice)
{
	AddCheckerRef(D3D12Texture, this);
	Context = inDevice;
	if (inDevice == nullptr)
	{
		Device = D3D12RHI::DXConv(RHI::GetDefaultDevice());
	}
	else
	{
		Device = D3D12RHI::DXConv(inDevice);
	}
	FrameCreated = RHI::GetFrameCount();
	if (FrameCreated == 0)
	{
		FrameCreated = -10;
	}
}

unsigned char * D3D12Texture::GenerateMip(int& startwidth, int& startheight, int bpp, unsigned char * StartData, int&mipsize, float ratio)
{
	std::string rpath = Engine::GetExecutionDir();
	rpath.append("\\asset\\DerivedDataCache\\");
	if (!FileUtils::File_ExistsTest(rpath))
	{
		PlatformApplication::TryCreateDirectory(rpath);
	}

	StringUtils::RemoveChar(TextureName, "\\asset\\texture\\");
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
	if (FileUtils::File_ExistsTest(rpath))
	{
		if (ImageIO::LoadTexture2D(rpath.c_str(), &buffer, &width, &height, &nChannels) != E_IMAGEIO_SUCCESS)
		{
			return buffer;
		}
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
	MipCreationTime += ((float)(endtime - StartTime) / 1e6f);
	return finalbuffer;
}

bool D3D12Texture::CLoad(AssetPathRef name)
{
	unsigned char *buffer = NULL;
	int nChannels;
	TextureName = name.BaseName;
	TexturePath = name.GetRelativePathToAsset();
	if (name.GetFileType() == AssetFileType::DDS || name.IsDDC)
	{
		return LoadDDS(name.GetFullPathToAsset());
	}
	else
	{
		if (ImageIO::LoadTexture2D(name.GetFullPathToAsset().c_str(), &buffer, &Width, &Height, &nChannels) != E_IMAGEIO_SUCCESS)
		{
			return false;
		}
	}

#if USE_CPUFALLBACK_TOGENMIPS_ATRUNTIME
	if (Width == 0 || Height == 0)
	{
		return;
	}
	unsigned char*  finalbuffer = GenerateMips(Miplevels - 1, Width, Height, buffer);
	MipLevelsReadyNow = Miplevels;
#else 
	unsigned char*  finalbuffer = buffer;
#endif
	CreateTextureFromData(finalbuffer, 0, Width, Height, 4);
	delete finalbuffer;
	return true;
}

bool D3D12Texture::LoadDDS(std::string filename)
{
	UsingDDSLoad = true;
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	bool IsCubeMap = false;
	DirectX::DDS_ALPHA_MODE ALPHA_MODE;
	HRESULT hr = DirectX::LoadDDSTextureFromFile(Device->GetDevice(), StringUtils::ConvertStringToWide(filename).c_str(), &m_texture, ddsData, subresources, 0ui64, &ALPHA_MODE, &IsCubeMap);
	if (hr != S_OK)
	{
		return false;
	}
	if (IsCubeMap)
	{
		CurrentTextureType = ETextureType::Type_CubeMap;
		MipLevelsReadyNow = (int)subresources.size() / 6;
	}
	else
	{
		CurrentTextureType = ETextureType::Type_2D;
		MipLevelsReadyNow = (int)subresources.size();
	}

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture, 0, (UINT)subresources.size());
	ID3D12Resource* textureUploadHeap = nullptr;
	format = m_texture->GetDesc().Format;
	// Create the GPU upload buffer.
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap)));
	NAME_D3D12_OBJECT(textureUploadHeap);
	UpdateSubresources(Device->GetCopyList(), m_texture, textureUploadHeap, 0, 0, (UINT)subresources.size(), subresources.data());
	TextureResource = new GPUResource(m_texture, D3D12_RESOURCE_STATE_COPY_DEST, Device);
	m_texture->SetName(L"Loaded Texture");
	Device->NotifyWorkForCopyEngine();
	D3D12RHI::Get()->AddObjectToDeferredDeleteQueue(textureUploadHeap);
	UpdateSRV();
	return true;
}

void D3D12Texture::Release()
{
	SafeRelease(m_texture);
	//	SafeRelease(srvHeap);
	SafeRelease(TextureResource);
	IRHIResourse::Release();
	RemoveCheckerRef(D3D12Texture, this);
}

D3D12Texture::~D3D12Texture()
{}

bool D3D12Texture::CreateFromFile(AssetPathRef FileName)
{
	return CLoad(FileName);
}

void D3D12Texture::BindToSlot(D3D12CommandList* list, int slot)
{
	if (RHI::GetFrameCount() > FrameCreated + 1)
	{
		if (list->IsGraphicsList())
		{
			TextureResource->SetResourceState(list->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			list->GetCommandList()->SetGraphicsRootDescriptorTable(slot, SRVDesc->GetGPUAddress());//ask the current heap to bind us
		}
		else if (list->IsComputeList() || list->IsRaytracingList())
		{
			list->GetCommandList()->SetComputeRootDescriptorTable(slot, SRVDesc->GetGPUAddress());
		}
	}
}

void D3D12Texture::CreateTextureFromDesc(const TextureDescription& desc)
{
	Description = desc;
	GPUResource* textureUploadHeap;
	// Describe and create a Texture2D.
	D3D12_RESOURCE_DESC textureDesc = {};

	textureDesc.MipLevels = desc.MipLevels;
	textureDesc.Width = desc.Width;
	textureDesc.Height = desc.Height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	if (UsingDDSLoad)
	{
		textureDesc.Format = format;
	}
	else
	{
		textureDesc.Format = D3D12Helpers::ConvertFormat(desc.Format);
	}
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	AllocDesc D;
	D.ResourceDesc = textureDesc;
	D.InitalState = D3D12_RESOURCE_STATE_COPY_DEST;
	Device->GetMemoryManager()->AllocTexture(D, &TextureResource);
	m_texture = TextureResource->GetResource();
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture, 0, MipLevelsReadyNow);

	D = AllocDesc();
	D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	D.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
	Device->GetMemoryManager()->AllocTemporary(D, &textureUploadHeap);
	D3D12Helpers::NameRHIObject(textureUploadHeap, this, "(UPLOAD)");


	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = desc.PtrToData;
	textureData.RowPitch = desc.Width * desc.BitDepth;
	textureData.SlicePitch = textureData.RowPitch * desc.Height;
	Texturedatarray[0] = textureData;
	UpdateSubresources(Device->GetCopyList(), m_texture, textureUploadHeap->GetResource(), 0, 0, MipLevelsReadyNow, &Texturedatarray[0]);

	RHI::AddToDeferredDeleteQueue(textureUploadHeap);
	Device->NotifyWorkForCopyEngine();
	m_texture->SetName(L"Texture");
	textureUploadHeap->SetName(L"Upload");
	// Describe and create a SRV for the texture.
	UpdateSRV();
}

void D3D12Texture::UpdateSRV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	ZeroMemory(&srvDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (UsingDDSLoad)
	{
		srvDesc.Format = format;
	}
	else
	{
		srvDesc.Format = D3D12Helpers::ConvertFormat(Description.Format);
	}
	if (MaxMip != -1)
	{
		MipLevelsReadyNow = 1;
	}
	if (CurrentTextureType == ETextureType::Type_CubeMap)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = MipLevelsReadyNow;
		srvDesc.TextureCube.MostDetailedMip = 0;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = MipLevelsReadyNow;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}

#if 0
	//test for streaming data like mips of disc!
	const int testmip = 8;
	if (MipLevelsReadyNow > testmip)
	{

		srvDesc.Texture2D.MipLevels = MipLevelsReadyNow - testmip;
		srvDesc.Texture2D.MostDetailedMip = testmip;
	}
#endif
	//create descriptor
	//add to heap
	if (SRVDesc == nullptr)
	{
		SRVDesc = Device->GetHeapManager()->AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	SRVDesc->CreateShaderResourceView(m_texture, &srvDesc);
}

ID3D12Resource * D3D12Texture::GetResource()
{
	return m_texture;
}

bool D3D12Texture::CheckDevice(int index)
{
	if (Device != nullptr)
	{
		return (Device->GetDeviceIndex() == index);
	}
	return false;
}

DescriptorGroup * D3D12Texture::GetDescriptor()
{
	return SRVDesc;
}

DXDescriptor * D3D12Texture::GetDescriptor(RHIViewDesc Desc)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	ZeroMemory(&srvDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (UsingDDSLoad)
	{
		srvDesc.Format = format;
	}
	else
	{
		srvDesc.Format = D3D12Helpers::ConvertFormat(Description.Format);
	}
	if (MaxMip != -1)
	{
		MipLevelsReadyNow = 1;
	}
	if (CurrentTextureType == ETextureType::Type_CubeMap)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = MipLevelsReadyNow;
		srvDesc.TextureCube.MostDetailedMip = 0;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = MipLevelsReadyNow;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}
	DXDescriptor* output = new DXDescriptor();
	output = Device->GetHeapManager()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	

	output->CreateShaderResourceView(m_texture, &srvDesc);
	output->Recreate();
	return output;
}

void D3D12Texture::CreateAsNull()
{
	//ensure(srvHeap == nullptr);
	//srvHeap = new DescriptorHeap(Device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	//srvHeap->SetName(L"Texture SRV");
	UpdateSRV();
}
