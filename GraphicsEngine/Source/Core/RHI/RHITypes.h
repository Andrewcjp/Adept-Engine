#pragma once
#include "Core/Types/FString.h"
#include "Core/IRefCount.h"

class Shader;
class FrameBuffer;
class DeviceContext;
#define MRT_MAX 8
#define NAME_RHI_PRIMS !BUILD_SHIPPING
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
namespace GPU_RESOURCE_STATES
{
	enum Type
	{
		RESOURCE_STATE_COMMON = 0,
		RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
		RESOURCE_STATE_INDEX_BUFFER = 0x2,
		RESOURCE_STATE_RENDER_TARGET = 0x4,
		RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
		RESOURCE_STATE_DEPTH_WRITE = 0x10,
		RESOURCE_STATE_DEPTH_READ = 0x20,
		RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
		RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
		RESOURCE_STATE_STREAM_OUT = 0x100,
		RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
		RESOURCE_STATE_COPY_DEST = 0x400,
		RESOURCE_STATE_COPY_SOURCE = 0x800,
		RESOURCE_STATE_RESOLVE_DEST = 0x1000,
		RESOURCE_STATE_RESOLVE_SOURCE = 0x2000,
		RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		RESOURCE_STATE_PRESENT = 0,
		RESOURCE_STATE_PREDICATION = 0x200,
		RESOURCE_STATE_VIDEO_DECODE_READ = 0x10000,
		RESOURCE_STATE_VIDEO_DECODE_WRITE = 0x20000,
		RESOURCE_STATE_VIDEO_PROCESS_READ = 0x40000,
		RESOURCE_STATE_VIDEO_PROCESS_WRITE = 0x80000,
		RESOURCE_STATE_UNDEFINED = 0x90000
	};
}
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
		RayTracing,
		Copy,
		VideoEncode,
		VideoDecode,
		Limit
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
		PreZ,
		DeferredWrite,
		DeferredLighting,
		UI,
		Text,
		Skybox,
		PostProcess,
		Present,
		ParticleDraw,
		ParticleSimulation,
		ShadowPreSample,
		GPU0WaitOnGPU1,
		CubemapCapture,
		DebugRender,
		RT_Trace,
		LIMIT
	};
}
namespace EGPUCOPYTIMERS
{
	enum Type
	{
		Total,
		MGPUCopy,
		ShadowCopy,
		ShadowCopy2,
		SFRMerge,
		LIMIT
	};
}
namespace ERHIBufferType
{
	enum Type
	{
		Vertex,
		Index,
		Constant,
		GPU
	};
}

//Render passes
struct ERenderPassStoreOp
{
	enum Type
	{
		Store,
		DontCare,
		Limit
	};
};

struct ERenderPassLoadOp
{
	enum Type
	{
		Load,
		Clear,
		DontCare,
		Limit
	};
};

//#RHI remove this?
struct RHI_API RHIPipeRenderTargetDesc
{
	eTEXTURE_FORMAT RTVFormats[8] = { eTEXTURE_FORMAT::FORMAT_UNKNOWN };
	eTEXTURE_FORMAT DSVFormat = eTEXTURE_FORMAT::FORMAT_UNKNOWN;
	int NumRenderTargets = 0;
	bool operator==(const RHIPipeRenderTargetDesc other) const;
};
enum TMP_BlendMode
{
	Text,
	Full
};

//defines a render pass pointers to framebuffer optional
struct RHIRenderPassDesc
{
	RHIRenderPassDesc() {}
	RHIRenderPassDesc(FrameBuffer* buffer, ERenderPassLoadOp::Type LoadOp = ERenderPassLoadOp::Load);
	FrameBuffer* TargetBuffer = nullptr;
	FrameBuffer* DepthSourceBuffer = nullptr;

	ERenderPassLoadOp::Type LoadOp = ERenderPassLoadOp::Clear;
	ERenderPassStoreOp::Type StoreOp = ERenderPassStoreOp::Store;

	ERenderPassLoadOp::Type StencilLoadOp = ERenderPassLoadOp::DontCare;
	ERenderPassStoreOp::Type StencilStoreOp = ERenderPassStoreOp::DontCare;

	GPU_RESOURCE_STATES::Type InitalState = GPU_RESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET;
	GPU_RESOURCE_STATES::Type FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET;
	RHIPipeRenderTargetDesc RenderDesc;
	RHI_API void Build();
	RHI_API bool operator==(const RHIRenderPassDesc other)const;

	bool TargetSwapChain = false;
};

struct RHIRender_Target_Blend_Desc
{

};

struct RHIBlendState
{
	bool AlphaToCoverageEnable = false;
	bool IndependentBlendEnable = false;
	RHIRender_Target_Blend_Desc RenderTargetDescs[MRT_MAX] = {};
};
struct RHIRasterizerDesc
{
	bool Cull = false;
};
struct RHIDepthStencilDesc
{
	bool DepthEnable = true;
	bool DepthWrite = true;
	/*DEPTH_WRITE_MASK DepthWriteMask;
	COMPARISON_FUNC DepthFunc;
	BOOL StencilEnable;
	UINT8 StencilReadMask;
	UINT8 StencilWriteMask;
	DEPTH_STENCILOP_DESC FrontFace;
	DEPTH_STENCILOP_DESC BackFace;*/
};

struct ViewInstancingMode
{
	bool Active = false;
	int Instances = 6;
};

struct  RHIPipeLineStateDesc
{
	//Hold both root signature and shader blobs
	Shader* ShaderInUse = nullptr;
	FrameBuffer* FrameBufferTarget = nullptr;
	void InitOLD(bool Depth, bool shouldcull, bool Blend);
	bool Cull = true;
	bool Blending = false;
	TMP_BlendMode Mode = Text;
	RHIBlendState BlendState;
	RHIRasterizerDesc RasterizerState;
	PRIMITIVE_TOPOLOGY_TYPE RasterMode = PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	COMPARISON_FUNC DepthCompareFunction = COMPARISON_FUNC::COMPARISON_FUNC_LESS;
	RHIPipeRenderTargetDesc RenderTargetDesc = RHIPipeRenderTargetDesc();
	RHIDepthStencilDesc DepthStencilState;
	int SampleCount = 1;

	//Validation in cases without a validation layer or engine limitations (e.g. multi-GPU)
	RHI_API bool Validate();
	DeviceContext* OwningDevice = nullptr;
	RHI_API size_t GetHash();
	RHI_API void CalulateHash();
	RHI_API bool operator==(const RHIPipeLineStateDesc other)const;
	static RHIPipeLineStateDesc CreateDefault(Shader* shader, FrameBuffer* FB = nullptr)
	{
		RHIPipeLineStateDesc desc;
		desc.ShaderInUse = shader;
		desc.FrameBufferTarget = FB;
		return desc;
	}
	RHI_API void Build();
	RHI_API std::string GetString();
	class RHIRenderPass* RenderPass = nullptr;
	ViewInstancingMode ViewInstancing;
	RHIRenderPassDesc RenderPassDesc;
	bool EnableDepthBoundsTest = false;
private:
	size_t UniqueHash = 0;
	std::string StringPreHash;
};

class  RHIPipeLineStateObject
{
public:
	RHI_API RHIPipeLineStateObject(const RHIPipeLineStateDesc& desc);
	RHI_API virtual ~RHIPipeLineStateObject();
	RHI_API size_t GetDescHash();
	RHI_API std::string GetDescString();
	RHI_API virtual void Complie();
	RHI_API bool IsReady() const;
	RHI_API const RHIPipeLineStateDesc& GetDesc();
	RHI_API DeviceContext* GetDevice()const;
	RHI_API virtual void Release();
protected:
	bool IsComplied = false;
	RHIPipeLineStateDesc Desc;
protected:
	DeviceContext* Device = nullptr;
};

namespace EFrameBufferSizeMode
{
	enum Type
	{
		Fixed,//Resize is handled explicitly 
		LinkedToRenderScale, //the Scale of the renderer
		LinkedToScreenSize, //Linked to the screen size
		Limit
	};
};

struct RHIFrameBufferDesc
{
public:
	static RHIFrameBufferDesc CreateColour(int width, int height);
	static RHIFrameBufferDesc CreateDepth(int width, int height);
	static RHIFrameBufferDesc CreateCubeDepth(int width, int height);
	static RHIFrameBufferDesc CreateCubeColourDepth(int width, int height);
	TEMP_API static RHIFrameBufferDesc CreateColourDepth(int width, int height);
	static RHIFrameBufferDesc CreateGBuffer(int width, int height);
	RHIFrameBufferDesc()
	{};
	RHIFrameBufferDesc(int width, int height, eTEXTURE_FORMAT format, eTextureDimension dimension)
	{
		RTFormats[0] = format;
		Width = width;
		Height = height;
		Dimension = dimension;
		MaxSize.x = width;
		MaxSize.y = height;
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
	bool CubeMapAddressAsOne = true;
	GPU_RESOURCE_STATES::Type StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_COMMON;
	glm::vec4 ViewPort = glm::vec4();
	glm::vec4 ScissorRect = glm::vec4();
	bool IncludedInSFR = false;
	float LinkToBackBufferScaleFactor = 1.0f;
	int SFR_FullWidth = 0;
	RHIPipeRenderTargetDesc GetRTDesc();
	bool AllowDynamicResize = false;
	glm::ivec2 MaxSize = glm::ivec2(0, 0);
	FrameBuffer* SharedDepthStencilSource = nullptr;
	EFrameBufferSizeMode::Type SizeMode = EFrameBufferSizeMode::Fixed;
};

class RHI_API IRHIResourse : public IRefCount
{
public:
	virtual ~IRHIResourse();
	virtual void Release();
	bool IsPendingKill() const
	{
		return PendingKill;
	}
	bool IsReleased = false;
	void SetDebugName(std::string Name);
	const char* ObjectSufix = "";
#if NAME_RHI_PRIMS	
	const char* GetDebugName();
#endif
	virtual size_t GetSizeOnGPU();
private:
#if NAME_RHI_PRIMS
	const char* FinalName = "";
	const char* DebugName = "";
#endif
	bool PendingKill = false;
	friend class RHI;

};
//Releases the GPU side and deletes the CPU object
#define SafeRHIRelease(Target) if(Target != nullptr){Target->Release(); delete Target; Target= nullptr;}
#define EnqueueSafeRHIRelease(Target) if(Target != nullptr){RHI::AddToDeferredDeleteQueue(Target);}
#define EnqueueSafeArrayRelease(x,length) for(int i= 0; i < length;i++){EnqueueSafeRHIRelease(x[i]);}
#if NAME_RHI_PRIMS
#define NAME_RHI_OBJECT(x) x->SetDebugName(#x);
#else
#define NAME_RHI_OBJECT(x)
#endif
struct IndirectDrawArgs
{
	int VertexCountPerInstance;
	int InstanceCount;
	int StartVertexLocation;
	int StartInstanceLocation;
};

struct IndirectDispatchArgs
{
	int ThreadGroupCountX;
	int ThreadGroupCountY;
	int ThreadGroupCountZ;
};

struct SFRNode
{
	float SFR_PercentSize = 0.5f;
	float SFR_VerticalPercentSize = 1.0f;
	float SFR_Offset = 0.5f;
	float SFR_VerticalOffset = 1.0f;
	struct TimerData* SFRDriveTimer = nullptr;
	std::vector<FrameBuffer*> NodesBuffers;
	void AddBuffer(FrameBuffer* b);
	//Link to framebuffers - then call resize!
};

struct RHITextureDesc
{
	bool InitOnALLDevices = true;
	bool IsCubeMap = false;
};

//Interface for multiple objects linking to the same "object" (texture or buffer) on different devices
template <class T>
class IRHISharedDeviceObject
{
public:
	virtual ~IRHISharedDeviceObject()
	{
		//for (int i = 0; i < SharedObjects.size(); i++)
		//{
		//	EnqueueSafeRHIRelease(SharedObjects[i]);
		//}
	}
	//#RHI_Multi: ref count both sides for now just link
	void RegisterOtherDeviceTexture(T * Other)
	{
		SharedObjects.push_back(Other);
	}
	T * GetOnOtherDevice(DeviceContext * Con)
	{
		for (int i = 0; i < SharedObjects.size(); i++)
		{
			if (Con == SharedObjects[i]->GetContext())
			{
				return SharedObjects[i];
			}
		}
		return nullptr;
	}
	template<class R>
	static R* GetObject(T* buffer, DeviceContext* Device)
	{
		if (!(((R*)buffer)->CheckDevice(Device->GetDeviceIndex())))
		{
			//Hack!
			T* otherdevicebuffer = buffer->GetOnOtherDevice(Device);
			if (otherdevicebuffer == nullptr)
			{
				Log::LogMessage("Failed to Bind object on Device", Log::Error);
				return (R*)buffer;
			}
			return (R*)otherdevicebuffer;
		}
		return (R*)buffer;
	}
private:
	std::vector<T*> SharedObjects;
};



namespace EBufferAccessType
{
	enum Type
	{
		Static,
		Dynamic,
		GPUOnly
	};
};

class RHI_API RHIUtils
{
public:
	static size_t BitsPerPixel(eTEXTURE_FORMAT fmt);
	static size_t GetPixelSize(eTEXTURE_FORMAT format);
};


namespace ESamplerWrapMode
{
	enum Type
	{
		TEXTURE_ADDRESS_MODE_WRAP = 1,
		TEXTURE_ADDRESS_MODE_MIRROR = 2,
		TEXTURE_ADDRESS_MODE_CLAMP = 3,
		TEXTURE_ADDRESS_MODE_BORDER = 4,
		TEXTURE_ADDRESS_MODE_MIRROR_ONCE = 5
	};
};

namespace ESamplerFilterMode
{
	enum Type
	{
		FILTER_MIN_MAG_MIP_POINT = 0,
		FILTER_MIN_MAG_POINT_MIP_LINEAR = 0x1,
		FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
		FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x5,
		FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x10,
		FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
		FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x14,
		FILTER_MIN_MAG_MIP_LINEAR = 0x15,
		FILTER_ANISOTROPIC = 0x55,
		FILTER_COMPARISON_MIN_MAG_MIP_POINT = 0x80,
		FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 0x81,
		FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x84,
		FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 0x85,
		FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 0x90,
		FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x91,
		FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 0x94,
		FILTER_COMPARISON_MIN_MAG_MIP_LINEAR = 0x95,
		FILTER_COMPARISON_ANISOTROPIC = 0xd5,
		FILTER_MINIMUM_MIN_MAG_MIP_POINT = 0x100,
		FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x101,
		FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x104,
		FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x105,
		FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x110,
		FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x111,
		FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x114,
		FILTER_MINIMUM_MIN_MAG_MIP_LINEAR = 0x115,
		FILTER_MINIMUM_ANISOTROPIC = 0x155,
		FILTER_MAXIMUM_MIN_MAG_MIP_POINT = 0x180,
		FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR = 0x181,
		FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x184,
		FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR = 0x185,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT = 0x190,
		FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x191,
		FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT = 0x194,
		FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR = 0x195,
		FILTER_MAXIMUM_ANISOTROPIC = 0x1d5
	};
};


struct RHISamplerDesc
{
	ESamplerFilterMode::Type FilterMode = ESamplerFilterMode::FILTER_MIN_MAG_MIP_LINEAR;
	ESamplerWrapMode::Type UAddressMode = ESamplerWrapMode::TEXTURE_ADDRESS_MODE_WRAP;
	ESamplerWrapMode::Type VAddressMode = ESamplerWrapMode::TEXTURE_ADDRESS_MODE_WRAP;
	ESamplerWrapMode::Type WAddressMode = ESamplerWrapMode::TEXTURE_ADDRESS_MODE_WRAP;
	int ShaderRegister = 0;
	int MaxAnisotropy = 0;
	RHISamplerDesc()
	{}
	RHISamplerDesc(ESamplerFilterMode::Type filter, ESamplerWrapMode::Type WrapMode, int Reg);
	RHI_API static std::vector<RHISamplerDesc> GetDefault();
};



struct RHIRayDispatchDesc
{
	RHIRayDispatchDesc() {};
	RHIRayDispatchDesc(FrameBuffer* RB);
	int Width = 0;
	int Height = 0;
	int Depth = 1;
	FrameBuffer* Target = nullptr;
};


