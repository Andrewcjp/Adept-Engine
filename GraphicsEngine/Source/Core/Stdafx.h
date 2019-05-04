#ifdef PCHGUARD
#error PCHGUARD Failure
#endif
#define PCHGUARD
#pragma once
#define NOMINMAX
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#pragma warning(push,3)
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/scalar_multiplication.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_major_storage.hpp>
#include <glm/gtc/integer.hpp>
#include <ios>
#include <iostream>
#include <map>
#include <memory.h>
#include <queue>
#include <random>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#pragma warning(pop)
#if 0
#pragma warning(push,4)
#pragma warning(push,4)
#pragma warning(disable:4100)//C4189
#pragma warning(disable:4505)//this is detect false positives
#pragma warning(disable:4238)
#endif
#if 1
#pragma warning(3:4189)
#pragma warning(3:4458)
#pragma warning(3:4457)
#pragma warning(3:4456)
#pragma warning(3:4189)
#pragma warning(3:4703)

//#pragma warning(3:4100)
#pragma warning(3:4265)
#pragma warning(3:4596)
//#pragma warning(3:4464)
#pragma warning(3:4700)
#pragma warning(3:4703)
//Unreachable code
//#pragma warning(3:4702)
#endif//C4150 incomplete ptr

//Start Engine Headers
#include "EngineGlobals.h"
#include "Core/Platform/PlatformTypes.h"
#include "Core/Asserts.h"

