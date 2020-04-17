#pragma once
#include "Dx12Select.h"


#if NTDDI_WIN10_RS4
#define WIN10_1803 1
#else
#define WIN10_1803 0 
#endif
#if NTDDI_WIN10_RS5
#define WIN10_1809 1
#else
#define WIN10_1809 0 
#endif
#ifdef NTDDI_WIN10_19H1
#define WIN10_1903 1
#define WIN10_1903_WIN 1
#else 
#define WIN10_1903 0 
#define WIN10_1903_WIN 0
#endif
#include "Stdafx.h"
#if CHECK_INCLUDE( "../ExtraPlatforms/D3D12RHIDefines.h")
#include "../ExtraPlatforms/D3D12RHIDefines.h"
#endif
#ifndef OVERRIDE_DX_CONFIG
typedef ID3D12GraphicsCommandList CopyCMDListType;
#endif

class GPUResource;

#ifdef PLATFORM_WINDOWS
#define SUPPORT_DXGI 1
#include "d3dx12.h" 
#endif

#if RHI_SUPPORTS_RT
#define USE_DIXL 1
#else
#define USE_DIXL 0
#endif

#ifndef OVERRIDE_DX_CONFIG
struct D3D12RHIConfig
{
	static const uint64 RenderTargetMemoryAlignment = 0;
	static const D3D12_HEAP_FLAGS IndirectBufferHeapFlag = D3D12_HEAP_FLAG_NONE;
	static const D3D12_RESOURCE_FLAGS IndirectBufferResouceFlag = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	static const D3D12_COMMAND_LIST_TYPE CopyListQueueType = D3D12_COMMAND_LIST_TYPE_COPY;
	static const D3D12_INDIRECT_ARGUMENT_TYPE DispatchArgOverride = D3D12_INDIRECT_ARGUMENT_TYPE::D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
	static const D3D12_ROOT_SIGNATURE_FLAGS RTRootSigExtraFlag = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE;//Some platforms need an extra flags for global root sigs
};
#define ID_PASS(x) IID_PPV_ARGS(x)
#include "RHI/RenderAPIs/D3D12/Platform/Windows/D3D12WindowSwapChain.h"
typedef D3D12WindowSwapChain SwapchainInferface;
#include "RHI/RenderAPIs/D3D12/Platform/Windows/WindowsDx12DeviceInterface.h"
typedef WindowsDx12DeviceInterface DX12DeviceInterface;
#endif


