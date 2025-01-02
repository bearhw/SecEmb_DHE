#include <torch/extension.h>

#include <iostream>
#include <string>

static inline float cmov_f32(uint32_t pred, float val1, float val2)
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

static inline uint64_t cmov_i64(uint64_t pred, uint64_t val1, uint64_t val2)
{
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

torch::Tensor Argmax_forward(torch::Tensor input_tensor)
{
    // std::cout << " ARGMAX FWD " << input_tensor.sizes() << "  \n";

    auto ind_a = input_tensor.accessor<float, 2>();

    int B = input_tensor.sizes()[0];
    int L = input_tensor.sizes()[1];

    uint64_t *output = new uint64_t[B * sizeof(uint64_t)];

    // TODO    THREAD     // # ,'-DAT_PARALLEL_OPENMP', '-fopenmp']),

    for (int b = 0; b < B; b++)
    {
        uint64_t max_idx = -1;
        float max_val = ind_a[b][0]; // first val init
        for (int i = 0; i < L; i++)
        {
            float curr_val = ind_a[b][i];

            bool OBLIVIOUS = true;

            // leaky
            if (!OBLIVIOUS)
                if (curr_val > max_val)
                {
                    max_val = curr_val;
                    max_idx = i;
                }

            // obliv
            if (OBLIVIOUS)
            {
                uint32_t flag = curr_val > max_val;
                max_val = cmov_f32(flag, curr_val, max_val);
                max_idx = cmov_i64(flag, i, max_idx);
            }
        }
        output[b] = max_idx;
    }

    return torch::from_blob(output, {B}, torch::TensorOptions().dtype(torch::kInt64)); // output dim B not {B, 1} to match HF generate()
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
    m.def("forward", &Argmax_forward, "Forward Argmax");
}
