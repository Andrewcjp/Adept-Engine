#pragma once
#include <d3d12.h>
#include <DXGI1_4.h>
#include "d3dx12.h"
#include "Stdafx.h"
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
#if RHI_SUPPORTS_RT
#define USE_DIXL 1
#else
#define USE_DIXL 0
#endif
#define SUPPORT_OPENVR 0
class GPUResource;