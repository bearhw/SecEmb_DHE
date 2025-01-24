#include <torch/extension.h>
#include "immintrin.h"

#include <iostream>
#include <string>

#ifdef FLAG_RELU_NONSECURE

// non-secure base ver
torch::Tensor ReLU_forward(torch::Tensor input_tensor)
{
    auto ind_a = input_tensor.accessor<float, 2>();

    int B = input_tensor.sizes()[0];
    int L = input_tensor.sizes()[1];

    float *output = (float *)malloc(B * L * sizeof(float));

    for (int b = 0; b < B; b++)
    {
        for (int i = 0; i < L; i++)
        {
            float curr_val = ind_a[b][i];

            output[b * L + i] = curr_val > 0 ? curr_val : 0;
        }
    }

    return torch::from_blob(output, {B, L});
}

#endif

static inline float cmov(uint32_t pred, float val1, float val2)
{
    // put new in result, if flag was zero, put old in result
    float result;
    __asm__(
        "mov %2, %0\n\t" // mov src dst
        "test %1, %1\n\t"
        "cmovz %3, %0\n\t"
        : [output] "=&r"(result) // & means early clobber
        : [input] "r"(pred), "r"(val1), "r"(val2)
        : "cc");
    return result;
}

#ifdef FLAG_RELU_OBLIVIOUS

torch::Tensor ReLU_forward(torch::Tensor input_tensor)
{
    int B = input_tensor.sizes()[0];
    int L = input_tensor.sizes()[1];

    float *output = (float *)malloc(B * L * sizeof(float));

    auto ind_a = input_tensor.accessor<float, 2>();

    uint32_t vec_len_avx512 = L * sizeof(float) / 64; // 64B in AVX512
    uint32_t simd_done_elem = vec_len_avx512 * 64 / sizeof(float);

    // printf("DEBUG %d %d %llu %d %d \n\n", B, L, output, vec_len_avx512, simd_done_elem);

#pragma omp parallel for
    for (int b = 0; b < B; b++)
    {
        //// vec blocks
        for (uint32_t e = 0; e < vec_len_avx512; e++)
        {
            __m512 z = _mm512_loadu_ps((input_tensor.data_ptr<float>() + b * L) + e * 16); // 16 floats in one vector
            __m512 all_zeros = _mm512_setzero_ps();
            __m512 res = _mm512_max_ps(z, all_zeros);
            _mm512_storeu_ps(output + b * L + (e * 16), res);
        }

        //// remaining elements using cmov
        // for (int i = simd_done_elem; i < L; i++)
        // {
        //     float curr_val = ind_a[b][i];
        //     uint32_t flag = curr_val > 0;
        //     float res = cmov(flag, curr_val, 0);
        //     output[b * L + i] = res;
        // }

        //// remaining elements using masked loads/stores
        if (simd_done_elem < L)
        {
            __m512 z = _mm512_maskz_loadu_ps((1 << (L - simd_done_elem)) - 1, (input_tensor.data_ptr<float>() + b * L + simd_done_elem));
            __m512 all_zeros = _mm512_setzero_ps();
            __m512 res = _mm512_max_ps(z, all_zeros);
            _mm512_mask_storeu_ps(output + b * L + simd_done_elem, (1 << (L - simd_done_elem)) - 1, res);
        }
    }

    auto retval = torch::from_blob(output, {B, L});

    return retval;
}

#endif

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
    m.def("forward", &ReLU_forward, "Forward ReLU");
}
