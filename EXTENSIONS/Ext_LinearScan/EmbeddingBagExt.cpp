#include <torch/extension.h>
#include "immintrin.h"

#include <iostream>
#include <string>

extern "C" void omove_buffer(unsigned char *dest, unsigned char *source, uint32_t buffersize, uint32_t flag);

// LS v
static inline uint64_t cmov(int pred, uint64_t val1, uint64_t val2)
{
    // put new in result, if flag was zero, put old in result
    uint64_t result;
    __asm__(
        "mov %2, %0\n\t" // mov src dst
        "test %1, %1\n\t"
        "cmovz %3, %0\n\t"
        : [output] "=&r"(result) // & means early clobber
        : [input] "r"(pred), "r"(val1), "r"(val2)
        : "cc");
    return result;
}

// extern "C"

static inline void omove_buffer_NEW(unsigned char *dest, unsigned char *source, uint32_t buffersize, uint32_t flag)
{
    uint64_t *src = (uint64_t *)source;
    uint64_t *dst = (uint64_t *)dest;

    uint32_t count = buffersize / 8;
    uint64_t res;
    for (uint32_t i = 0; i < count; i++)
    {
        uint64_t orig = dst[i];
        uint64_t new_ = src[i];
        res = cmov(flag, new_, orig);
        dst[i] = res;
    }
}

static inline void omove_buffer_avx2(unsigned char *dest, unsigned char *source, uint32_t buffersize, uint32_t flag)
{
    // __m256i mask = _mm256_set1_epi64x(flag == 1 ? -1 : 0); // broadcast flag to mask, has condition though
    __m256i allzero = _mm256_set1_epi64x(0);
    __m256i flagb = _mm256_set1_epi64x((uint64_t)flag);
    __m256i mask = _mm256_cmpeq_epi64(allzero, flagb);
    mask = ~mask;                     // if flag is zero, then cmp-eq sets 0xffff, we need to negate it for blending, HENCE if flag not zero then mask is eventually all ones
    uint32_t count = buffersize / 32; // 256bits = 32bytes
    for (uint32_t i = 0; i < count; i++)
    {
        __m256i read1 = _mm256_loadu_si256((__m256i const *)source + i);
        __m256i read2 = _mm256_loadu_si256((__m256i const *)dest + i);
        __m256i res = _mm256_blendv_epi8(read2, read1, mask); // if flag
        _mm256_storeu_si256((__m256i *)dest + i, res);
    }
}

static inline void add_float32_avx2(float *farr1, float *farr2, int N)
{
    uint32_t count = N * sizeof(float) / 32; // 256bits = 32bytes
    for (uint32_t i = 0; i < count; i++)
    {
        __m256 read1 = _mm256_loadu_ps(farr1 + i * 8); // 32/4=8 floats
        __m256 read2 = _mm256_loadu_ps(farr2 + i * 8);
        __m256 res = _mm256_add_ps(read2, read1);
        _mm256_storeu_ps(farr1 + i * 8, res);
    }
}

static inline void omove_buffer_avx512(unsigned char *dest, unsigned char *source, uint32_t buffersize, uint32_t flag)
{
    // __m512i  mask = _mm256_set1_epi64x(flag == 1 ? -1 : 0); // broadcast flag to mask, has condition though
    __m512i allzero = _mm512_set1_epi64(0);
    __m512i flagb = _mm512_set1_epi64((uint64_t)flag);
    __mmask8 mask = _mm512_cmpeq_epi64_mask(allzero, flagb);
    // printf("flag %d mask %u\n", flag, mask);
    mask = ~mask; // if flag is zero, then cmp-eq sets 0xffff, we need to negate it for blending, HENCE if flag not zero then mask is eventually all ones
    // printf("mask %u\n", mask);
    uint32_t count = buffersize / 64; // 512bits = 64bytes
    for (uint32_t i = 0; i < count; i++)
    {
        __m512i read1 = _mm512_loadu_si512((__m512i const *)source + i);
        __m512i read2 = _mm512_loadu_si512((__m512i const *)dest + i);
        __m512i res = _mm512_mask_blend_epi64(mask, read2, read1); // Performs element-by-element blending of int64 source vectors a and b, using the instruction mask k as selector.
        _mm512_storeu_si512((__m512i *)dest + i, res);
    }
}

static inline void add_float32_avx512(float *farr1, float *farr2, int N)
{
    uint32_t count = N * sizeof(float) / 64; //
    for (uint32_t i = 0; i < count; i++)
    {
        __m512 read1 = _mm512_loadu_ps(farr1 + i * 16); // 64/4=16 floats
        __m512 read2 = _mm512_loadu_ps(farr2 + i * 16);
        __m512 res = _mm512_add_ps(read2, read1);
        _mm512_storeu_ps(farr1 + i * 16, res);
    }
}

struct EmbeddingBag
{
    EmbeddingBag(uint32_t num_embeddings, uint32_t embedding_dim, const std::string &mode_)
    {
        n = num_embeddings;
        m = embedding_dim;
        mode = mode_;

        table_weights = new float[m * n * sizeof(float)];
        weights = table_weights;
    }

    EmbeddingBag(uint32_t num_embeddings, uint32_t embedding_dim)
    {
        n = num_embeddings;
        m = embedding_dim;
        // mode = "";

        printf("LS EXT ========= %d %d\n", n, m);
        fflush(stdout);

        table_weights = new float[m * n * sizeof(float)];
        weights = table_weights;
    }

    void setWeights(torch::Tensor inData)
    {
        std::cout << " SETWGHT PRINT  " << m * n << "     " << inData.numel() << " " << inData.sizes() << "\n";

        // assert TO-DO dtype same, size same
        assert(m * n == inData.numel());

        std::memcpy(table_weights, inData.data_ptr(), sizeof(float) * inData.numel());

        fflush(stdout);
    }

    torch::Tensor forward(torch::Tensor indices, torch::Tensor offsets)
    {
        auto ind_a = indices.accessor<long, 1>();
        auto off_a = offsets.accessor<long, 1>();

        int B = offsets.sizes()[0];
        int IND = indices.sizes()[0];

        float *output = new float[B * m * sizeof(float)];
        memset(output, 0, B * m * sizeof(float));

        float *tmp_out;

#pragma omp parallel private(tmp_out)
        {
            tmp_out = new float[m * sizeof(float)];

#pragma omp for
            // #pragma omp parallel for
            for (int i = 0; i < B; i++)
            {
                uint32_t offset_start, offset_end;
                offset_start = off_a[i];
                if (i == B - 1)
                    offset_end = IND;
                else
                    offset_end = off_a[i + 1];

                for (uint32_t ii = offset_start; ii < offset_end; ii++)
                {
                    uint32_t idx = ind_a[ii];
                    uint32_t cond;

                    for (uint32_t k = 0; k < n; k++)
                    {
                        cond = (idx == k); // flag if match current index

                        omove_buffer_avx512((unsigned char *)(tmp_out), (unsigned char *)(table_weights + k * m), m * sizeof(float), cond);
                    }

                    add_float32_avx512(output + i * m, tmp_out, m);
                }
            }
        }
        return torch::from_blob(output, {B, m});
    }

    std::string mode;
    uint32_t n;
    uint32_t m;
    float *table_weights; // templated To-do?
    float *weights;       // remove
};

namespace py = pybind11;
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
    py::class_<EmbeddingBag>(m, "EmbeddingBag")
        .def(py::init<uint32_t, uint32_t, const std::string &>())
        .def(py::init<uint32_t, uint32_t>())
        .def("setWeights", &EmbeddingBag::setWeights)
        .def("forward", &EmbeddingBag::forward)
        .def("__call__", &EmbeddingBag::forward)
        ;
}
