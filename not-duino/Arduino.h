// we build for a lot of platforms and none of them are arduino

#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>

#ifdef __ARM_FEATURE_UNALIGNED
#define ALLOWS_UNALIGNED
#endif

#ifdef _MSC_VER
#define __builtin_bswap16(x) _byteswap_ushort(x)
#define __builtin_bswap32(x) _byteswap_ulong(x)
#endif
