#pragma once
typedef unsigned int uint;
typedef uint64_t uint64;
static_assert(sizeof(uint64) == 8, "wrong uint64 Size");
static_assert(sizeof(uint) == 4, "wrong uint Size");