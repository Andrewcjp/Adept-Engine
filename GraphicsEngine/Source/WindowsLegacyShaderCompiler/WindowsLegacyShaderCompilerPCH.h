#pragma once
#include "Stdafx.h"
#include <vector>
#include <functional>


#include <d3d12.h> 
#include <dxcapi.h>       // be sure to link with dxcompiler_x.lib
#include <d3d12shader.h>  // shader reflection

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
#else 
#define WIN10_1903 0 
#endif