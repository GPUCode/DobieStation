#pragma once
#include <cstdint>
#include <type_traits>
#include <limits>
#include <numeric>
#include <array>
#include <string_view>
#include <algorithm>

#define DOBIE_AVX2
#include <immintrin.h>

namespace simd
{
#ifdef DOBIE_AVX2
    template <typename T>
    using native_t = std::conditional_t<std::is_same_v<T, double>, __m256d,
        std::conditional_t<std::is_same_v<T, float>, __m256, __m256i>>;
#else
#error "ARM64 support, please implement"
#endif

    namespace priv
    {
        #define error(msg) [] <bool flag = false>() { static_assert(flag, msg); }()

#ifdef DOBIE_AVX2
        template <typename T, std::size_t S = sizeof(T)>
        auto inline broadcast_impl(const T& n)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_set1_epi16(n);
                else if constexpr (S == 4)
                    return _mm256_set1_epi32(n);
                else if constexpr (S == 8)
                    return _mm256_set1_epi64x(n);
                else
                    error("[SIMD] broadcast_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_set1_ps(n);
                else if constexpr (S == 8)
                    return _mm256_set1_pd(n);
                else
                    error("[SIMD] broadcast_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] broadcast_impl: Unknown type.");
            }
        }

        /* Load multiple elements from memory */
        template <typename T, std::size_t S = sizeof(T)>
        auto inline broadcast_multiple_impl(const void* data)
        {
            if constexpr (std::is_integral_v<T>)
            {
                return _mm256_loadu_si256(reinterpret_cast<const native_t<T>*>(data));
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_loadu_ps(reinterpret_cast<const float*>(data));
                else if constexpr (S == 8)
                    return _mm256_loadu_pd(reinterpret_cast<const double*>(data));
                else
                    error("[SIMD] broadcast_multiple_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] broadcast_multiple_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        void inline store_impl(const native_t<T>& a, void* memory)
        {
            if constexpr (std::is_integral_v<T>)
            {
                _mm256_storeu_si256(reinterpret_cast<native_t<T>*>(memory), a);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_storeu_ps(reinterpret_cast<float*>(memory), a);
                else if constexpr (S == 8)
                    return _mm256_storeu_pd(reinterpret_cast<double*>(memory), a);
                else
                    error("[SIMD] store_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] store_impl: Unknown type.");
            }
        }

        /* Vector addition */
        template <typename T, std::size_t S = sizeof(T)>
        auto inline add_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_add_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_add_epi16(a, b);
                else if constexpr (S == 4)
                    return _mm256_add_epi32(a, b);
                else if constexpr (S == 8)
                    return _mm256_add_epi64(a, b);
                else
                    error("[SIMD] add_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_add_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_add_pd(a, b);
                else
                    error("[SIMD] add_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] add_impl: Unknown type.");
            }
        }

        /* Vector subtraction */
        template <typename T, std::size_t S = sizeof(T)>
        auto inline sub_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_sub_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_sub_epi16(a, b);
                else if constexpr (S == 4)
                    return _mm256_sub_epi32(a, b);
                else if constexpr (S == 8)
                    return _mm256_sub_epi64(a, b);
                else
                    error("[SIMD] sub_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_sub_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_sub_pd(a, b);
                else
                    error("[SIMD] sub_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] sub_impl: Unknown type.");
            }
        }

        /* Vector multiplication */
        template <typename T, std::size_t S = sizeof(T)>
        auto inline mul_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_mullo_epi16(a, b);
                else if constexpr (S == 4 && std::is_signed_v<T>)
                    return _mm256_mul_epi32(a, b);
                else if constexpr (S == 4 && std::is_unsigned_v<T>)
                    return _mm256_mul_epu32(a, b);
                else
                    error("[SIMD] sub_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_mul_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_mul_pd(a, b);
                else
                    error("[SIMD] mul_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] mul_impl: Unknown type.");
            }
        }

        /* Vector division */
        template <typename T, std::size_t S = sizeof(T)>
        auto inline div_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                error("[SIMD] div_impl: Integer division unsupported.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_div_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_div_pd(a, b);
                else
                    error("[SIMD] div_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] div_impl: Unknown type.");
            }
        }

        /* Shift vector by constant or by another vector */
        template <typename T, std::size_t S = sizeof(T)>
        auto inline shift_left_impl(const native_t<T>& a, int shift)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_slli_epi16(a, shift);
                else if constexpr (S == 4)
                    return _mm256_slli_epi32(a, shift);
                else if constexpr (S == 8)
                    return _mm256_slli_epi64(a, shift);
                else
                    error("[SIMD] shift_left_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                error("[SIMD] shift_left_impl: Shifting floats is not supported.");
            }
            else
            {
                error("[SIMD] shift_left_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline shift_left_multiple_impl(const native_t<T>& a, const native_t<T>& shift)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_sllv_epi16(a, shift);
                else if constexpr (S == 4)
                    return _mm256_sllv_epi32(a, shift);
                else if constexpr (S == 8)
                    return _mm256_sllv_epi64(a, shift);
                else
                    error("[SIMD] shift_left_multiple_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                error("[SIMD] shift_left_multiple_impl: Shifting floats is not supported.");
            }
            else
            {
                error("[SIMD] shift_left_multiple_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline shift_right_impl(const native_t<T>& a, int shift)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_srai_epi16(a, shift);
                else if constexpr (S == 4)
                    return _mm256_srai_epi32(a, shift);
                else if constexpr (S == 8)
                    return _mm256_srai_epi64(a, shift);
                else
                    error("[SIMD] shift_right_impl: Unknown integral type.");
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_srli_epi16(a, shift);
                else if constexpr (S == 4)
                    return _mm256_srli_epi32(a, shift);
                else if constexpr (S == 8)
                    return _mm256_srli_epi64(a, shift);
                else
                    error("[SIMD] shift_right_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                error("[SIMD] shift_right_impl: Shifting floats is not supported.");
            }
            else
            {
                error("[SIMD] shift_right_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline shift_right_multiple_impl(const native_t<T>& a, const native_t<T>& shift)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_srav_epi16(a, shift);
                else if constexpr (S == 4)
                    return _mm256_srav_epi32(a, shift);
                else if constexpr (S == 8)
                    return _mm256_srav_epi64(a, shift);
                else
                    error("[SIMD] shift_right_multiple_impl: Unknown integral type.");
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                if constexpr (S == 2)
                    return _mm256_srlv_epi16(a, shift);
                else if constexpr (S == 4)
                    return _mm256_srlv_epi32(a, shift);
                else if constexpr (S == 8)
                    return _mm256_srlv_epi64(a, shift);
                else
                    error("[SIMD] shift_right_multiple_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                error("[SIMD] shift_right_multiple_impl: Shifting floats is not supported.");
            }
            else
            {
                error("[SIMD] shift_right_multiple_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline select_impl(const native_t<T>& a, const native_t<T>& b, const native_t<T>& mask)
        {
            if constexpr (std::is_integral_v<T>)
            {
                return _mm256_blendv_epi8(a, b, mask);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_blendv_ps(a, b, mask);
                else if constexpr (S == 8)
                    return _mm256_blendv_pd(a, b, mask);
                else
                    error("[SIMD] select_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] select_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline equal_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_cmpeq_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_cmpeq_epi16(a, b);
                else if constexpr (S == 4)
                    return _mm256_cmpeq_epi32(a, b);
                else if constexpr (S == 8)
                    return _mm256_cmpeq_epi64(a, b);
                else
                    error("[SIMD] equal_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_cmp_ps(a, b, _CMP_EQ_OQ);
                else if constexpr (S == 8)
                    return _mm256_cmp_pd(a, b, _CMP_EQ_OQ);
                else
                    error("[SIMD] equal_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] lesser_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline greater_impl(const native_t<T>& a, const native_t<T>& b);

        template <typename T, std::size_t S = sizeof(T)>
        auto inline lesser_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                return greater_impl<T>(b, a);
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_cmpeq_epi8(_mm256_max_epu8(b, a), b);
                else if constexpr (S == 2)
                    return _mm256_cmpeq_epi16(_mm256_max_epu16(b, a), b);
                else if constexpr (S == 4)
                    return _mm256_cmpeq_epi32(_mm256_max_epu32(b, a), b);
                else if constexpr (S == 8)
                    return _mm256_cmpeq_epi64(_mm256_max_epu64(b, a), b);
                else
                    error("[SIMD] lesser_impl: Unknown unsigned integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_cmp_ps(a, b, _CMP_LT_OQ);
                else if constexpr (S == 8)
                    return _mm256_cmp_pd(a, b, _CMP_LT_OQ);
                else
                    error("[SIMD] greater_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] greater_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S>
        auto inline greater_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_cmpgt_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_cmpgt_epi16(a, b);
                else if constexpr (S == 4)
                    return _mm256_cmpgt_epi32(a, b);
                else if constexpr (S == 8)
                    return _mm256_cmpgt_epi64(a, b);
                else
                    error("[SIMD] greater_impl: Unknown signed integral type.");
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_xor_si256(lesser_impl<T>(a, b), _mm256_set1_epi8(-1));
                else if constexpr (S == 2)
                    return _mm256_xor_si256(lesser_impl<T>(a, b), _mm256_set1_epi16(-1));
                else if constexpr (S == 4)
                    return _mm256_xor_si256(lesser_impl<T>(a, b), _mm256_set1_epi32(-1));
                else if constexpr (S == 8)
                    return _mm256_xor_si256(lesser_impl<T>(a, b), _mm256_set1_epi64x(-1));
                else
                    error("[SIMD] greater_impl: Unknown unsigned integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_cmp_ps(a, b, _CMP_GT_OQ);
                else if constexpr (S == 8)
                    return _mm256_cmp_pd(a, b, _CMP_GT_OQ);
                else
                    error("[SIMD] greater_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] greater_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline bitwise_and_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                return _mm256_and_si256(a, b);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_and_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_and_pd(a, b);
                else
                    error("[SIMD] bitwise_and_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] bitwise_and_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline bitwise_or_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                return _mm256_or_si256(a, b);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_or_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_or_pd(a, b);
                else
                    error("[SIMD] bitwise_or_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] bitwise_or_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline bitwise_xor_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                return _mm256_xor_si256(a, b);
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_xor_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_xor_pd(a, b);
                else
                    error("[SIMD] bitwise_xor_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] bitwise_xor_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline bitwise_not_impl(const native_t<T>& a)
        {
            /* This is what GCC generates */
            return bitwise_xor_impl<T>(a, equal_impl<T>(a, a));
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline min_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_min_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_min_epi16(a, b);
                else if constexpr (S == 4)
                    return _mm256_min_epi32(a, b);
                else if constexpr (S == 8)
                    return _mm256_min_epi64(a, b);
                else
                    error("[SIMD] min_impl: Unknown integral type.");
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_min_epu8(a, b);
                else if constexpr (S == 2)
                    return _mm256_min_epu16(a, b);
                else if constexpr (S == 4)
                    return _mm256_min_epu32(a, b);
                else if constexpr (S == 8)
                    return _mm256_min_epu64(a, b);
                else
                    error("[SIMD] min_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_min_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_min_pd(a, b);
                else
                    error("[SIMD] min_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] min_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline max_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_max_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_max_epi16(a, b);
                else if constexpr (S == 4)
                    return _mm256_max_epi32(a, b);
                else if constexpr (S == 8)
                    return _mm256_max_epi64(a, b);
                else
                    error("[SIMD] max_impl: Unknown integral type.");
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_max_epu8(a, b);
                else if constexpr (S == 2)
                    return _mm256_max_epu16(a, b);
                else if constexpr (S == 4)
                    return _mm256_max_epu32(a, b);
                else if constexpr (S == 8)
                    return _mm256_max_epu64(a, b);
                else
                    error("[SIMD] max_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_max_ps(a, b);
                else if constexpr (S == 8)
                    return _mm256_max_pd(a, b);
                else
                    error("[SIMD] max_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] max_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline shuffle_impl(const native_t<T>& a, const native_t<T>& mask)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_shuffle_epi8(a, mask);
                else if constexpr (S == 4)
                    return _mm256_shuffle_epi32(a, mask);
                else
                    error("[SIMD] shuffle_impl: Unknown integral type.");
            }
            else
            {
                error("[SIMD] shuffle_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline permute_impl(const native_t<T>& a, const native_t<T>& mask)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_permutevar8x32_epi32(a, mask);
                else
                    error("[SIMD] permute_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 4)
                    return _mm256_permutevar8x32_ps(a, mask);
                else
                    error("[SIMD] permute_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] permute_impl: Unknown type.");
            }
        }

        template <typename T, const int mask, std::size_t S = sizeof(T)>
        auto inline permute_impl(const native_t<T>& a)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 8)
                    return _mm256_permute4x64_epi64(a, mask);
                else
                    error("[SIMD] shuffle_impl: Unknown integral type.");
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if constexpr (S == 8)
                    return _mm256_permute4x64_pd(a, mask);
                else
                    error("[SIMD] shuffle_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] shuffle_impl: Unknown type.");
            }
        }

        template <typename To, typename From, std::size_t TO_S = sizeof(To), std::size_t FROM_S = sizeof(From)>
        auto inline extend_impl(const native_t<From>& a)
        {
            static_assert((std::is_integral_v<From> && std::is_integral_v<To>) ||
                (std::is_floating_point_v<From> && std::is_floating_point_v<To>),
                "[SIMD] Converting between float and int types not supported.");

            if constexpr (std::is_integral_v<From> && std::is_signed_v<From>)
            {
                if constexpr (FROM_S == 1)
                {
                    if constexpr (TO_S == 2)
                        return _mm256_cvtepi8_epi16(_mm256_castsi256_si128(a));
                    else if constexpr (TO_S == 4)
                        return _mm256_cvtepi8_epi32(_mm256_castsi256_si128(a));
                    else if constexpr (TO_S == 8)
                        return _mm256_cvtepi8_epi64(_mm256_castsi256_si128(a));
                    else
                        error("[SIMD] extend_impl: Unknown integral signed type.");
                }
                else if constexpr (FROM_S == 2)
                {
                    if constexpr (TO_S == 1)
                        return _mm256_zextsi128_si256(_mm256_cvtepi16_epi8(a));
                    else if constexpr (TO_S == 4)
                        return _mm256_cvtepi16_epi32(_mm256_castsi256_si128(a));
                    else if constexpr (TO_S == 8)
                        return _mm256_cvtepi16_epi64(_mm256_castsi256_si128(a));
                    else
                        error("[SIMD] extend_impl: Unknown integral signed type.");
                }
                else if constexpr (FROM_S == 4)
                {
                    if constexpr (TO_S == 1)
                        return _mm256_zextsi128_si256(_mm256_cvtepi32_epi8(a));
                    else if constexpr (TO_S == 2)
                        return _mm256_zextsi128_si256(_mm256_cvtepi32_epi16(a));
                    else if constexpr (TO_S == 8)
                        return _mm256_cvtepi32_epi64(_mm256_castsi256_si128(a));
                    else
                        error("[SIMD] extend_impl: Unknown integral signed type");
                }
            }
            else if constexpr (std::is_integral_v<From> && std::is_unsigned_v<From>)
            {
                if constexpr (FROM_S == 1)
                {
                    if constexpr (TO_S == 2)
                        return _mm256_cvtepu8_epi16(_mm256_castsi256_si128(a));
                    else if constexpr (TO_S == 4)
                        return _mm256_cvtepu8_epi32(_mm256_castsi256_si128(a));
                    else if constexpr (TO_S == 8)
                        return _mm256_cvtepu8_epi64(_mm256_castsi256_si128(a));
                    else
                        error("[SIMD] extend_impl: Unknown integral unsigned type.");
                }
                else if constexpr (FROM_S == 2)
                {
                    if constexpr (TO_S == 1)
                        return _mm256_zextsi128_si256(_mm256_cvtepu16_epi8(a));
                    else if constexpr (TO_S == 4)
                        return _mm256_cvtepu16_epi32(_mm256_castsi256_si128(a));
                    else if constexpr (TO_S == 8)
                        return _mm256_cvtepu16_epi64(_mm256_castsi256_si128(a));
                    else
                        error("[SIMD] extend_impl: Unknown integral unsigned type.");
                }
                else if constexpr (FROM_S == 4)
                {
                    if constexpr (TO_S == 1)
                        return _mm256_zextsi128_si256(_mm256_cvtepu32_epi8(a));
                    else if constexpr (TO_S == 2)
                        return _mm256_zextsi128_si256(_mm256_cvtepu32_epi16(a));
                    else if constexpr (TO_S == 8)
                        return _mm256_cvtepu32_epi64(_mm256_castsi256_si128(a));
                    else
                        error("[SIMD] extend_impl: Unknown integral unsigned type");
                }
            }
            else if constexpr (std::is_floating_point_v<From>)
            {
                if constexpr (FROM_S == 4 && TO_S == 8)
                    return _mm256_cvtps_pd(_mm256_castps256_ps128(a));
                else if constexpr (FROM_S == 8 && TO_S == 4)
                    return _mm256_castps128_ps256(_mm256_cvtpd_ps(a));
                else
                    error("[SIMD] extend_impl: Unknown float type.");
            }
            else
            {
                error("[SIMD] extend_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline abs_impl(const native_t<T>& a)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_abs_epi8(a);
                else if constexpr (S == 2)
                    return _mm256_abs_epi16(a);
                else if constexpr (S == 4)
                    return _mm256_abs_epi32(a);
                else if constexpr (S == 8)
                    return _mm256_abs_epi64(a);
                else
                    error("[SIMD] abs_impl: Unknown integral type.");
            }
            else
            {
                error("[SIMD] abs_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline neg_impl(const native_t<T>& a, const native_t<T>& mask)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_sign_epi8(a, mask);
                else if constexpr (S == 2)
                    return _mm256_sign_epi16(a, mask);
                else if constexpr (S == 4)
                    return _mm256_sign_epi32(a, mask);
                else if constexpr (S == 8)
                    return _mm256_sign_epi64(a, mask);
                else
                    error("[SIMD] neg_impl: Unknown integral type.");
            }
            else
            {
                error("[SIMD] neg_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline add_saturate_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_adds_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_adds_epi16(a, b);
                else
                    error("[SIMD] add_saturate_impl: Unknown integral type.");
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                auto resp = add_impl<T>(a, b);
                auto mask = greater_impl<T>(a, resp);
                return bitwise_or_impl<T>(resp, mask);
            }
            else
            {
                error("[SIMD] add_saturate_impl: Uknown type.");
            }
        }

        /* 32bit sature add need a custom implementation */
        template <>
        auto inline add_saturate_impl<int32_t, 4>(const __m256i& a, const __m256i& b)
        {
            const __m256i int_max = broadcast_impl<int32_t>(0x7FFFFFFF);
            __m256i res = add_impl<int32_t>(a, b);

            /* If result saturates, it has the same sign as both a and b */
            __m256i sign_bit = shift_right_impl<int32_t>(a, 31); // shift sign to lowest bit
            __m256i saturated = add_impl<int32_t>(int_max, sign_bit);

            /* Saturation happened if inputs do not have different signs,
               and the sign of the result is different */
            __m256i sign_xor = bitwise_xor_impl<int32_t>(a, b);
            __m256i overflow = _mm256_andnot_si256(sign_xor, bitwise_xor_impl<int32_t>(a, res));

            /* Cast to float and then back to get the result. */
            return _mm256_castps_si256(_mm256_blendv_ps(_mm256_castsi256_ps(res),
                _mm256_castsi256_ps(saturated),
                _mm256_castsi256_ps(overflow)));
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline sub_saturate_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                if constexpr (S == 1)
                    return _mm256_subs_epi8(a, b);
                else if constexpr (S == 2)
                    return _mm256_subs_epi16(a, b);
                else
                    error("[SIMD] sub_saturate_impl: Unknown integral type.");
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                auto resp = sub_impl<T>(a, b);
                auto mask = bitwise_not_impl<T>(greater_impl<T>(resp, a));
                return bitwise_and_impl<T>(resp, mask);
            }
            else
            {
                error("[SIMD] add_saturate_impl: Uknown type.");
            }
        }

        /* Specialize for signed 32bit elements */
        template <>
        auto inline sub_saturate_impl<int32_t, 4>(const __m256i& a, const __m256i& b)
        {
            return add_saturate_impl<int32_t>(a, _mm256_sub_epi32(_mm256_setzero_si256(), b));
        }

        template <typename To, typename From, std::size_t TO_S = sizeof(To), std::size_t FROM_S = sizeof(From)>
        auto inline pack_impl(const native_t<From>& a)
        {
            static_assert((std::is_integral_v<From> && std::is_integral_v<To>) ||
                (std::is_floating_point_v<From> && std::is_floating_point_v<To>),
                "[SIMD] pack_impl: From - To types must match!");

            if constexpr (std::is_integral_v<To> && std::is_unsigned_v<To>)
            {
                if constexpr (TO_S == 1 && FROM_S == 2)
                    return _mm256_packus_epi16(a, _mm256_setzero_si256());
                else if constexpr (TO_S == 2 && FROM_S == 4)
                    return _mm256_packus_epi32(a, _mm256_setzero_si256());
                else
                    error("[SIMD] pack_impl: Unknown unsigned integral type.");
            }
            else if constexpr (std::is_integral_v<To> && std::is_signed_v<To>)
            {
                if constexpr (TO_S == 1 && FROM_S == 2)
                    return _mm256_packs_epi16(a, _mm256_setzero_si256());
                else if constexpr (TO_S == 2 && FROM_S == 4)
                    return _mm256_packs_epi32(a, _mm256_setzero_si256());
                else
                    error("[SIMD] pack_impl: Unknown unsigned integral type.");
            }
            else
            {
                error("[SIMD] pack_impl: Unknown type.");
            }
        }

        template <typename T, std::size_t S = sizeof(T)>
        auto inline interleave_impl(const native_t<T>& a, const native_t<T>& b)
        {
            if constexpr (std::is_integral_v<T>)
            {
                __m256i r1, r2;
                if constexpr (S == 1)
                {
                    r1 = _mm256_unpacklo_epi8(a, b);
                    r2 = _mm256_unpackhi_epi8(a, b);
                }
                else if constexpr (S == 2)
                {
                    r1 = _mm256_unpacklo_epi16(a, b);
                    r2 = _mm256_unpackhi_epi16(a, b);
                }
                else if constexpr (S == 4)
                {
                    r1 = _mm256_unpacklo_epi32(a, b);
                    r2 = _mm256_unpackhi_epi32(a, b);
                }
                else if constexpr (S == 8)
                {
                    r1 = _mm256_unpacklo_epi64(a, b);
                    r2 = _mm256_unpackhi_epi64(a, b);
                }
                else
                    error("[SIMD] interleave_impl: Unknown integral type.");

                return _mm256_inserti128_si256(r1, _mm256_castsi256_si128(r2), 1);
            }
            else
            {
                error("[SIMD] interleave_impl: Unknown type.");
            }
        }
#endif
    }

    /* Portable 256bit vector abstraction */
    template <typename T>
    struct Vec256
    {
        Vec256() = default;
        ~Vec256() = default;

        /* Arithmetic operators */
        auto operator+(const Vec256<T>& v) const;
        auto operator+(const T& n) const;
        auto operator-(const Vec256<T>& v) const;
        auto operator-(const T& n) const;
        auto operator*(const Vec256<T>& v) const;
        auto operator*(const T& n) const;
        auto operator/(const Vec256<T>& v) const;
        auto operator/(const T& n) const;

        /* Comparison operators */
        auto operator<(const Vec256<T>& v) const;
        auto operator<(const T& n) const;
        auto operator<=(const Vec256<T>& v) const;
        auto operator<=(const T& n) const;
        auto operator>(const Vec256<T>& v) const;
        auto operator>(const T& n) const;
        auto operator>=(const Vec256<T>& v) const;
        auto operator>=(const T& n) const;
        auto operator==(const Vec256<T>& v) const;
        auto operator==(const T& n) const;

        /* Bitwise operators */
        auto operator>>(const Vec256<T>& v) const;
        auto operator>>(const T& b) const;
        auto operator<<(const Vec256<T>& v) const;
        auto operator<<(const T& n) const;
        auto operator|(const Vec256<T>& v) const;
        auto operator|(const T& n) const;
        auto operator&(const Vec256<T>& v) const;
        auto operator&(const T& n) const;
        auto operator!() const;

        native_t<T> _impl;
    };

    /* Fill the vector with a single element */
    template <typename T>
    Vec256<T> fill(const T& n)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::broadcast_impl<T>(n);
        return vec;
    }

    template <typename T, typename ...Ts>
    constexpr std::array<T, sizeof...(Ts)> make_array(Ts... args)
    {
        return { {static_cast<T>(args)...} };
    }

    /* Broadcast array of elements in vector */
    template <typename T, typename... Args>
    constexpr Vec256<T> inline broadcast(Args... args)
    {
        auto elems = make_array<T>(args...);

        auto vec = Vec256<T>();
        vec._impl = priv::broadcast_multiple_impl<T>(elems.data());
        return vec;
    }

    /* Load data to vector from a block of memory or using C++ initializer lists */
    template <typename T>
    constexpr Vec256<T> inline load(const void* elems)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::broadcast_multiple_impl<T>(elems);
        return vec;
    }

    template <typename T>
    constexpr Vec256<T> inline load(const std::initializer_list<T>& elems)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::broadcast_multiple_impl<T>(std::data(elems));
        return vec;
    }

    template <typename T>
    constexpr void inline store(const Vec256<T>& a, void* memory)
    {
        priv::store_impl<T>(a._impl, memory);
    }

    /* Constructs a vector whose elements are either from a or b depending on the
       sign bit of each element in mask. */
    template <typename T>
    Vec256<T> select(const Vec256<T>& a, const Vec256<T>& b, const Vec256<T>& mask)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::select_impl<T>(a._impl, b._impl, mask._impl);
        return vec;
    }

    template <typename T>
    Vec256<T> select(const Vec256<T>& a, const T& b, const Vec256<T>& mask)
    {
        auto vec = Vec256<T>();
        auto false_case = fill<T>(b);

        vec._impl = priv::select_impl<T>(a._impl, false_case._impl, mask._impl);
        return vec;
    }

    /* Chnages how operations view the vector. Shouldn't generate any instructions */
    template <typename To, typename From>
    constexpr Vec256<To> inline reinterpret(const Vec256<From>& a)
    {
        auto vec = Vec256<To>();
        vec._impl = a._impl;
        return vec;
    }

    /* Extends each element by either zero or sign extending. Upper data will be lost. */
    template <typename To, typename From>
    constexpr Vec256<To> inline extend(const Vec256<From>& a)
    {
        auto vec = Vec256<To>();
        vec._impl = priv::extend_impl<To, From>(a._impl);
        return vec;
    }

    /* Compares each element of a and b and stores the minimun. Signness is automatically handled */
    template <typename T>
    constexpr Vec256<T> inline min(const Vec256<T>& a, const Vec256<T>& b)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::min_impl<T>(a, b);
        return vec;
    }

    /* Compares each element of a and b and stores the maximum. Signness is automatically handled */
    template <typename T>
    constexpr Vec256<T> inline max(const Vec256<T>& a, const Vec256<T>& b)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::max_impl<T>(a._impl, b._impl);
        return vec;
    }

    /* Shuffles vector elements according to the mask. Limited to 128bit range */
    template <typename T>
    constexpr Vec256<T> inline shuffle(const Vec256<T>& a, const Vec256<T>& mask)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::shuffle_impl<T>(a._impl, mask._impl);
        return vec;
    }

    /* Shuffles vector elements across the 128bit boundary. Word aligned */
    template <typename T>
    constexpr Vec256<T> inline permute(const Vec256<T>& a, const Vec256<T>& mask)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::permute_impl<T>(a._impl, mask._impl);
        return vec;
    }

    /* Shuffles vector elements across the 128bit boundary. Uses a compile time mask instead of a vector */
    template <const int mask, typename T>
    constexpr Vec256<T> inline permute(const Vec256<T>& a)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::permute_impl<T, mask>(a._impl);
        return vec;
    }

    template <typename T>
    constexpr Vec256<T> inline abs(const Vec256<T>& a)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::abs_impl<T>(a._impl);
        return vec;
    }

    /* Negates elements of the source vector according to the mask.
       a[i] = (mask[i] < 0 ? -a[i] : (mask[i] > 0 ? a[i] : 0)) */
    template <typename T>
    constexpr Vec256<T> inline negate(const Vec256<T>& a, const Vec256<T>& mask)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::neg_impl<T>(a._impl, mask._impl);
        return vec;
    }

    /* Negates all elemens of the source vector.
       a[i] = -a[i] */
    template <typename T>
    constexpr Vec256<T> inline negate(const Vec256<T>& a)
    {
        auto mask = fill<T>(-1);
        auto vec = Vec256<T>();
        vec._impl = priv::neg_impl<T>(a._impl, mask._impl);
        return vec;
    }

    template <typename T>
    constexpr Vec256<T> inline add_saturated(const Vec256<T>& a, const Vec256<T>& b)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::add_saturate_impl<T>(a._impl, b._impl);
        return vec;
    }

    template <typename T>
    constexpr Vec256<T> inline sub_saturated(const Vec256<T>& a, const Vec256<T>& b)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::sub_saturate_impl<T>(a._impl, b._impl);
        return vec;
    }

    /* Packs elemets of size <From> to size <To> using saturation. Limited to 128bit range. */
    template <typename To, typename From>
    constexpr Vec256<To> inline pack(const Vec256<From>& a)
    {
        auto vec = Vec256<To>();
        vec._impl = priv::pack_impl<To, From>(a._impl);
        return vec;
    }

    /* Interleaves the lower 128bit of a and b */ 
    template <typename T>
    constexpr Vec256<T> inline interleave(const Vec256<T>& a, const Vec256<T>& b)
    {
        auto vec = Vec256<T>();
        vec._impl = priv::interleave_impl<T>(a._impl, b._impl);
        return vec;
    }

    /* Implement vector operator overloading */
    template<typename T>
    auto Vec256<T>::operator+(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::add_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator+(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator+(vec);
    }

    template<typename T>
    auto Vec256<T>::operator-(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::sub_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator-(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator-(vec);
    }

    template<typename T>
    auto Vec256<T>::operator*(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::mul_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator*(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator*(vec);
    }

    template<typename T>
    auto Vec256<T>::operator/(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::div_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator/(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator/(vec);
    }

    template<typename T>
    auto Vec256<T>::operator>>(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::shift_right_multiple_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator>>(const T& n) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::shift_right_impl<T>(_impl, n);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator<<(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::shift_left_multiple_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator<<(const T& n) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::shift_left_impl<T>(_impl, n);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator|(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::bitwise_or_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator|(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator|(vec);
    }

    template<typename T>
    auto Vec256<T>::operator&(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::bitwise_and_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator& (const T& n) const
    {
        auto vec = fill<T>(n);
        return operator&(vec);
    }

    template<typename T>
    auto Vec256<T>::operator!() const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::bitwise_not_impl<T>(_impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator<(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::greater_impl<T>(v._impl, _impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator<(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator<(vec);
    }

    template<typename T>
    auto Vec256<T>::operator<=(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        /* (this <= v) => !(this > v) */
        vec._impl = priv::bitwise_not_impl<T>(priv::greater_impl<T>(_impl, v._impl));
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator<=(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator<=(vec);
    }

    template<typename T>
    auto Vec256<T>::operator>(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::greater_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator>(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator>(vec);
    }

    template<typename T>
    auto Vec256<T>::operator>=(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        /* (this >= v) => !(v > this) */
        vec._impl = priv::bitwise_not_impl<T>(priv::greater_impl<T>(v._impl, _impl));
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator>=(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator>=(vec);
    }

    template<typename T>
    auto Vec256<T>::operator==(const Vec256<T>& v) const
    {
        auto vec = Vec256<T>();
        vec._impl = priv::equal_impl<T>(_impl, v._impl);
        return vec;
    }

    template<typename T>
    auto Vec256<T>::operator==(const T& n) const
    {
        auto vec = fill<T>(n);
        return operator==(vec);
    }
}

namespace util
{
    /* Clamps a batch of signed 32bit values */
    inline auto clamp_halfword_parallel(simd::Vec256<int32_t>& a)
    {
        /* word > (int32_t)0x00007FFF */
        auto mask1 = a > (int32_t)0x00007FFF;
        auto result = simd::select(a, simd::fill<int32_t>(0x7FFF), mask1);

        /* word < (int32_t)0xFFFF80000 */
        auto mask2 = a < (int32_t)0xFFFF80000;
        result = simd::select(result, simd::fill<int32_t>(0x8000), mask2);

        /* (int16_t)word */
        return result;
    }

    /* Clamps a batch of signed 64bit values */
    inline auto clamp_doubleword_parallel(simd::Vec256<int64_t>& a)
    {
        /* word >= (int64_t)0x7FFFFFFF */
        auto mask1 = a >= (int64_t)0x7FFFFFFF;
        auto result = simd::select(a, simd::fill<int64_t>(0x7FFFFFFF), mask1);

        /* word <= (int64_t)0xFFFFFFFF80000000 */
        auto mask2 = a <= (int64_t)0xFFFFFFFF80000000;
        result = simd::select(result, simd::fill<int64_t>(0xFFFFFFFF80000000), mask2);

        /* (int64_t)(int32_t)word */
        return result & 0xFFFFFFFF;
    }
}