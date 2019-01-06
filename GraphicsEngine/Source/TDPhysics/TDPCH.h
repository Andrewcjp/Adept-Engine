#pragma once
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#if defined(TDPHYSICS_EXPORT) 
#   define TD_API DLLEXPORT
#else 
#ifndef STATIC_MODULE
#   define TD_API DLLIMPORT
#else 
#   define TD_API
#endif
#endif // TD_API
#include <vector>
#ifdef BUILD_SHIP
#define BUILD_FULLRELEASE 1
#else
#define BUILD_FULLRELEASE 0
#endif
#define SafeDelete(Target)if(Target != nullptr){delete Target; Target= nullptr;}
#define SafeRelease(Target) if(Target != nullptr){Target->Release(); Target= nullptr;}
#include "TDAsserts.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <functional>

#define VALIDATE_KE 0
#define BUILD_DEBUG_RENDER 1
#define ALLOW_ROT 0
