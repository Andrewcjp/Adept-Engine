#pragma once
#include <d3d12.h>
#include <DXGI1_4.h>
#include "d3dx12.h"
#include "Stdafx.h"
#if RHI_SUPPORTS_RT
#define USE_DIXL 1
#else
#define USE_DIXL 0
#endif