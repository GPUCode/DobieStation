#pragma once
#ifdef _WIN32
#include <intrin.h>
#endif

namespace util
{
	inline int count_leading_zeros(uint32_t value)
	{
#ifdef _WIN32
        unsigned long leading_zero = 0;
        bool zero = _BitScanReverse(&leading_zero, value);
        return 31 - (leading_zero + zero);
#else
        return (value != 0 ? __builtin_clz(value) - 1 : 0x1f);
#endif
	}
}