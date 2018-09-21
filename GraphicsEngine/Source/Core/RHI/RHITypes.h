#pragma once

#include <d3d12.h>
#define MRT_MAX 8
enum eTextureDimension
{
	DIMENSION_UNKNOWN = 0,
	DIMENSION_BUFFER = 1,
	DIMENSION_TEXTURE1D = 2,
	DIMENSION_TEXTURE1DARRAY = 3,
	DIMENSION_TEXTURE2D = 4,
	DIMENSION_TEXTURE2DARRAY = 5,
	DIMENSION_TEXTURE2DMS = 6,
	DIMENSION_TEXTURE2DMSARRAY = 7,
	DIMENSION_TEXTURE3D = 8,
	DIMENSION_TEXTURECUBE = 9,
	DIMENSION_TEXTURECUBEARRAY = 10
};
enum eTEXTURE_FORMAT
{
	FORMAT_UNKNOWN = 0,
	FORMAT_R32G32B32A32_TYPELESS = 1,
	FORMAT_R32G32B32A32_FLOAT = 2,
	FORMAT_R32G32B32A32_UINT = 3,
	FORMAT_R32G32B32A32_SINT = 4,
	FORMAT_R32G32B32_TYPELESS = 5,
	FORMAT_R32G32B32_FLOAT = 6,
	FORMAT_R32G32B32_UINT = 7,
	FORMAT_R32G32B32_SINT = 8,
	FORMAT_R16G16B16A16_TYPELESS = 9,
	FORMAT_R16G16B16A16_FLOAT = 10,
	FORMAT_R16G16B16A16_UNORM = 11,
	FORMAT_R16G16B16A16_UINT = 12,
	FORMAT_R16G16B16A16_SNORM = 13,
	FORMAT_R16G16B16A16_SINT = 14,
	FORMAT_R32G32_TYPELESS = 15,
	FORMAT_R32G32_FLOAT = 16,
	FORMAT_R32G32_UINT = 17,
	FORMAT_R32G32_SINT = 18,
	FORMAT_R32G8X24_TYPELESS = 19,
	FORMAT_D32_FLOAT_S8X24_UINT = 20,
	FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	FORMAT_R10G10B10A2_TYPELESS = 23,
	FORMAT_R10G10B10A2_UNORM = 24,
	FORMAT_R10G10B10A2_UINT = 25,
	FORMAT_R11G11B10_FLOAT = 26,
	FORMAT_R8G8B8A8_TYPELESS = 27,
	FORMAT_R8G8B8A8_UNORM = 28,
	FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	FORMAT_R8G8B8A8_UINT = 30,
	FORMAT_R8G8B8A8_SNORM = 31,
	FORMAT_R8G8B8A8_SINT = 32,
	FORMAT_R16G16_TYPELESS = 33,
	FORMAT_R16G16_FLOAT = 34,
	FORMAT_R16G16_UNORM = 35,
	FORMAT_R16G16_UINT = 36,
	FORMAT_R16G16_SNORM = 37,
	FORMAT_R16G16_SINT = 38,
	FORMAT_R32_TYPELESS = 39,
	FORMAT_D32_FLOAT = 40,
	FORMAT_R32_FLOAT = 41,
	FORMAT_R32_UINT = 42,
	FORMAT_R32_SINT = 43,
	FORMAT_R24G8_TYPELESS = 44,
	FORMAT_D24_UNORM_S8_UINT = 45,
	FORMAT_R24_UNORM_X8_TYPELESS = 46,
	FORMAT_X24_TYPELESS_G8_UINT = 47,
	FORMAT_R8G8_TYPELESS = 48,
	FORMAT_R8G8_UNORM = 49,
	FORMAT_R8G8_UINT = 50,
	FORMAT_R8G8_SNORM = 51,
	FORMAT_R8G8_SINT = 52,
	FORMAT_R16_TYPELESS = 53,
	FORMAT_R16_FLOAT = 54,
	FORMAT_D16_UNORM = 55,
	FORMAT_R16_UNORM = 56,
	FORMAT_R16_UINT = 57,
	FORMAT_R16_SNORM = 58,
	FORMAT_R16_SINT = 59,
	FORMAT_R8_TYPELESS = 60,
	FORMAT_R8_UNORM = 61,
	FORMAT_R8_UINT = 62,
	FORMAT_R8_SNORM = 63,
	FORMAT_R8_SINT = 64,
	FORMAT_A8_UNORM = 65,
	FORMAT_R1_UNORM = 66,
	FORMAT_R9G9B9E5_SHAREDEXP = 67,
	FORMAT_R8G8_B8G8_UNORM = 68,
	FORMAT_G8R8_G8B8_UNORM = 69,
	FORMAT_BC1_TYPELESS = 70,
	FORMAT_BC1_UNORM = 71,
	FORMAT_BC1_UNORM_SRGB = 72,
	FORMAT_BC2_TYPELESS = 73,
	FORMAT_BC2_UNORM = 74,
	FORMAT_BC2_UNORM_SRGB = 75,
	FORMAT_BC3_TYPELESS = 76,
	FORMAT_BC3_UNORM = 77,
	FORMAT_BC3_UNORM_SRGB = 78,
	FORMAT_BC4_TYPELESS = 79,
	FORMAT_BC4_UNORM = 80,
	FORMAT_BC4_SNORM = 81,
	FORMAT_BC5_TYPELESS = 82,
	FORMAT_BC5_UNORM = 83,
	FORMAT_BC5_SNORM = 84,
	FORMAT_B5G6R5_UNORM = 85,
	FORMAT_B5G5R5A1_UNORM = 86,
	FORMAT_B8G8R8A8_UNORM = 87,
	FORMAT_B8G8R8X8_UNORM = 88,
	FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	FORMAT_B8G8R8A8_TYPELESS = 90,
	FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	FORMAT_B8G8R8X8_TYPELESS = 92,
	FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	FORMAT_BC6H_TYPELESS = 94,
	FORMAT_BC6H_UF16 = 95,
	FORMAT_BC6H_SF16 = 96,
	FORMAT_BC7_TYPELESS = 97,
	FORMAT_BC7_UNORM = 98,
	FORMAT_BC7_UNORM_SRGB = 99,
	FORMAT_AYUV = 100,
	FORMAT_Y410 = 101,
	FORMAT_Y416 = 102,
	FORMAT_NV12 = 103,
	FORMAT_P010 = 104,
	FORMAT_P016 = 105,
	FORMAT_420_OPAQUE = 106,
	FORMAT_YUY2 = 107,
	FORMAT_Y210 = 108,
	FORMAT_Y216 = 109,
	FORMAT_NV11 = 110,
	FORMAT_AI44 = 111,
	FORMAT_IA44 = 112,
	FORMAT_P8 = 113,
	FORMAT_A8P8 = 114,
	FORMAT_B4G4R4A4_UNORM = 115,
	FORMAT_FORCE_UINT = 0xffffffff
};

enum PRIMITIVE_TOPOLOGY_TYPE
{
	PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED = 0,
	PRIMITIVE_TOPOLOGY_TYPE_POINT = 1,
	PRIMITIVE_TOPOLOGY_TYPE_LINE = 2,
	PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE = 3,
	PRIMITIVE_TOPOLOGY_TYPE_PATCH = 4
};
namespace DeviceContextQueue
{
	enum Type
	{
		Graphics,
		Compute,
		Copy,//used to move resources from Host to this GPU
		InterCopy,//Used to Transfer Resources From other GPUS Via HOST
		LIMIT
	};
}
namespace EBufferResourceState
{
	enum Type
	{
		UnorderedAccess,
		Read,
		IndirectArgs,
		Limit
	};
}
namespace ECommandListType
{
	enum Type
	{
		Graphics,
		Compute,
		Copy
	};
}

enum COMPARISON_FUNC
{
	COMPARISON_FUNC_NEVER = 1,
	COMPARISON_FUNC_LESS = 2,
	COMPARISON_FUNC_EQUAL = 3,
	COMPARISON_FUNC_LESS_EQUAL = 4,
	COMPARISON_FUNC_GREATER = 5,
	COMPARISON_FUNC_NOT_EQUAL = 6,
	COMPARISON_FUNC_GREATER_EQUAL = 7,
	COMPARISON_FUNC_ALWAYS = 8
};
namespace EGPUTIMERS
{
	enum Type
	{
		Total,
		PointShadows,
		DirShadows,
		MainPass,
		DeferredWrite,
		DeferredLighting,
		UI,
		Text,
		Skybox,
		PostProcess,
		Present,
		ShadowPreSample,
		ParticleDraw,
		ParticleSimulation,
		LIMIT
	};
}
namespace EGPUCOPYTIMERS
{
	enum Type
	{
		Total,
		MGPUCopy,
		LIMIT
	};
}
struct RHI_API RHIPipeRenderTargetDesc
{
	eTEXTURE_FORMAT RTVFormats[8] = { eTEXTURE_FORMAT::FORMAT_UNKNOWN };
	eTEXTURE_FORMAT DSVFormat = eTEXTURE_FORMAT::FORMAT_UNKNOWN;
	int NumRenderTargets = 0;
};

struct RHI_API PipeLineState
{
	bool DepthTest = true;
	bool Cull = true;
	bool Blending = false;
	PRIMITIVE_TOPOLOGY_TYPE RasterMode = PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	COMPARISON_FUNC DepthCompareFunction = COMPARISON_FUNC::COMPARISON_FUNC_LESS;
	bool DepthWrite = true;
	RHIPipeRenderTargetDesc RenderTargetDesc = RHIPipeRenderTargetDesc();
};

struct RHIFrameBufferDesc
{
public:
	static RHIFrameBufferDesc CreateColour(int width, int height);
	static RHIFrameBufferDesc CreateDepth(int width, int height);
	static RHIFrameBufferDesc CreateCubeDepth(int width, int height);
	static RHIFrameBufferDesc CreateCubeColourDepth(int width, int height);
	static RHIFrameBufferDesc CreateColourDepth(int width, int height);
	static RHIFrameBufferDesc CreateGBuffer(int width, int height);
	RHIFrameBufferDesc()
	{};
	RHIFrameBufferDesc(int width, int height, eTEXTURE_FORMAT format, eTextureDimension dimension)
	{
		RTFormats[0] = format;
		Width = width;
		Height = height;
		Dimension = dimension;
	}
	eTEXTURE_FORMAT RTFormats[MRT_MAX] = {};
	eTEXTURE_FORMAT DepthFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	eTEXTURE_FORMAT DepthReadFormat = eTEXTURE_FORMAT::FORMAT_R32_FLOAT;
	int Width = 0;
	int Height = 0;
	int TextureDepth = 1;
	int RenderTargetCount = 1;
	bool NeedsDepthStencil = false;
	bool IsShared = false;
	class DeviceContext* DeviceToCopyTo = nullptr;
	bool AllowUnordedAccess = false;
	//If set to 0 the resource will be auto mipped
	int MipsToGenerate = 1;
	eTextureDimension Dimension = eTextureDimension::DIMENSION_TEXTURE2D;
	glm::vec4 clearcolour = glm::vec4(0.0f, 0.2f, 0.4f, 1.0f);
	float DepthClearValue = 1.0f;
	int Samples = 0;
	int MipCount = 1;
	int DepthMipCount = 1;
	D3D12_RESOURCE_STATES StartingState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
};

class RHI_API IRHIResourse
{
public:
	virtual ~IRHIResourse();
	virtual void Release();
	bool IsPendingKill()
	{
		return PendingKill;
	}
	bool IsReleased = false;
private:
	bool PendingKill = false;
	friend class RHI;
};
//Releases the GPU side and deletes the CPU object
#define SafeRHIRelease(Target) if(Target != nullptr){Target->Release(); delete Target; Target= nullptr;}
#define EnqueueSafeRHIRelease(Target) if(Target != nullptr){RHI::AddToDeferredDeleteQueue(Target);}
