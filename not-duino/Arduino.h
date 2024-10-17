// we build for a lot of platforms and none of them are arduino

#pragma once
#include <cstdint>
#include <cstring>

#ifdef __ARM_FEATURE_UNALIGNED
#define ALLOWS_UNALIGNED
#endif

