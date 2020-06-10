#pragma once
typedef unsigned int uint;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint32_t Bool;
typedef unsigned char	uint8;
static_assert(sizeof(uint64) == 8, "wrong uint64 Size");
static_assert(sizeof(uint) == 4, "wrong uint Size");
static_assert(sizeof(uint32) == 4, "wrong uint32 Size");
static_assert(sizeof(Bool) == 4, "wrong Bool Size");