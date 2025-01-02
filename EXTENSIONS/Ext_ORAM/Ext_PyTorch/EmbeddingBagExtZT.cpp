#include <torch/extension.h>
#include "immintrin.h"

#include <iostream>
#include <string>
#include <chrono>

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

// ???
extern "C" void omove_buffer(unsigned char *dest, unsigned char *source, uint32_t buffersize, uint32_t flag);

#include "ZT_header.hpp"
#include "../ZT_Utils/PathORAM_Enclave.hpp"
#include "../ZT_Utils/CircuitORAM_Enclave.hpp"
extern "C" std::vector<PathORAM *> poram_instances;
extern "C" std::vector<CircuitORAM *> coram_instances;

// #define STASH_PRINT 1

struct EmbeddingBag
{
    std::string mode;
    uint32_t n;
    uint32_t m;

    uint32_t MAX_BLOCKS;
    uint32_t DATA_SIZE;
    uint32_t STASH_SIZE;
    uint32_t OBLIVIOUS_FLAG;
    uint32_t RECURSION_DATA_SIZE;
    uint32_t ORAM_TYPE;
    uint32_t Z;
    uint32_t zt_id;

    void print_stash_count(uint32_t instance_id)
    {
        uint32_t stash_oc = -1;
        if (ORAM_TYPE == 0)
            stash_oc = poram_instances[instance_id]->recursive_stash[0].stashOccupancy();
        else if (ORAM_TYPE == 1)
            stash_oc = coram_instances[instance_id]->recursive_stash[0].stashOccupancy();
        printf("stash_oc : %d\n", stash_oc);
        fflush(stdout);
    }

    EmbeddingBag(uint32_t num_embeddings, uint32_t embedding_dim, const std::string &mode_)
    {
        printf("\n\n\n\nCREATING ZT New %d %d\n", num_embeddings, embedding_dim);
        n = num_embeddings;
        m = embedding_dim;
        mode = mode_;
        fflush(stdout);

        MAX_BLOCKS = num_embeddings;
        DATA_SIZE = embedding_dim * sizeof(float);
        OBLIVIOUS_FLAG = 1;
        RECURSION_DATA_SIZE = 64;

#ifdef FLAG_ZT_PO
        ORAM_TYPE = 0; // path
        STASH_SIZE = 150;
        Z = 4;
#endif

#ifdef FLAG_ZT_CO
        ORAM_TYPE = 1; // circuit
        STASH_SIZE = 10;
        Z = 4;
#endif

        printf("Params = STASH_SIZE %d Z %d\n", STASH_SIZE, Z);
        zt_id = ZT_New(MAX_BLOCKS, DATA_SIZE, STASH_SIZE, OBLIVIOUS_FLAG, RECURSION_DATA_SIZE, ORAM_TYPE, Z);
        printf("zt_id = %d \n", zt_id);
        fflush(stdout);
    }

    EmbeddingBag(uint32_t num_embeddings, uint32_t embedding_dim) : EmbeddingBag(num_embeddings, embedding_dim, "sum")
    {
        
    }

    void setWeights(torch::Tensor inData)
    {
        std::cout << " SETWGHT PRINT  " << m * n << "     " << inData.numel() << " " << inData.sizes() << "\n";

#ifdef FLAG_NO_SETWEIGHT
        printf("SKIP SET WEIGHT\n");
        return;
#endif
        fflush(stdout);

        // assert TO-DO dtype same, size same
        assert(m * n == inData.numel());

        // fflush(stdout);

        uint32_t request_size, response_size;
        request_size = ID_SIZE_IN_BYTES + DATA_SIZE;
        response_size = DATA_SIZE;
        unsigned char *sample_data_in;
        unsigned char *sample_data_out;
        sample_data_in = (unsigned char *)malloc(DATA_SIZE);
        sample_data_out = (unsigned char *)malloc(DATA_SIZE + 1);
        unsigned char *serialized_request = (unsigned char *)malloc(1 + ID_SIZE_IN_BYTES + DATA_SIZE);

        RandomRequestSource reqsource;
        uint32_t *rand_seq = reqsource.GenerateRandomPermutation(MAX_BLOCKS);

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // randomize valid blocks
        for (int j = 0; j < MAX_BLOCKS; j++)
        {
            // printf("\n");
            int idx = rand_seq[j];
            serializeRequest(idx, 'r', sample_data_in, 4, serialized_request); // small data size doesnt matter>
            ZT_Access(zt_id, ORAM_TYPE, serialized_request, sample_data_out, request_size, response_size);
            // printf("Read: %d %d %u\n", j, idx, (uint32_t)sample_data_out[0]);

#ifdef STASH_PRINT
            print_stash_count(zt_id);
#endif
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Randomize blocks Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
        fflush(stdout);

        begin = std::chrono::steady_clock::now();
        // load weights
        for (int j = 0; j < MAX_BLOCKS; j++)
        {
            // printf("\n");
            int idx = j;
            unsigned char *curr_wgt = ((unsigned char *)inData.data_ptr()) + j * DATA_SIZE;
            serializeRequest(idx, 'w', curr_wgt, DATA_SIZE, serialized_request);
            ZT_Access(zt_id, ORAM_TYPE, serialized_request, sample_data_out, request_size, response_size);
            // printf("Read: %d %d %u\n", j, idx, (uint32_t)sample_data_out[0]);

#ifdef STASH_PRINT
            print_stash_count(zt_id);
#endif
        }
        end = std::chrono::steady_clock::now();
        std::cout << "Load weights " << MAX_BLOCKS << " -- Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
        fflush(stdout);

        printf("setWeight done \n");
        fflush(stdout);
    }

    torch::Tensor forward(torch::Tensor indices, torch::Tensor offsets)
    {
        uint32_t request_size, response_size;
        request_size = ID_SIZE_IN_BYTES + DATA_SIZE;
        response_size = DATA_SIZE;
        unsigned char *sample_data_in;
        unsigned char *sample_data_out;
        sample_data_in = (unsigned char *)malloc(DATA_SIZE); // TO-DO one time only !
        sample_data_out = (unsigned char *)malloc(DATA_SIZE + 1);
        unsigned char *serialized_request = (unsigned char *)malloc(1 + ID_SIZE_IN_BYTES + DATA_SIZE);

        auto ind_a = indices.accessor<long, 1>();
        auto off_a = offsets.accessor<long, 1>();

        int B = offsets.sizes()[0];
        int IND = indices.sizes()[0];
        float *output = new float[B * m * sizeof(float)];

        memset(output, 0, B * m * sizeof(float));
        float *tmp_out = new float[m * sizeof(float)];
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
                serializeRequest(idx, 'r', sample_data_in, 4, serialized_request);
                ZT_Access(zt_id, ORAM_TYPE, serialized_request, (unsigned char *)tmp_out, request_size, response_size);
                add_float32_avx2(output + i * m, tmp_out, m);
            }
        }

        fflush(stdout);
        return torch::from_blob(output, {B, m});
    }
};

namespace py = pybind11;
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
    py::class_<EmbeddingBag>(m, "EmbeddingBag", py::module_local())
        .def(py::init<uint32_t, uint32_t, const std::string &>())
        .def(py::init<uint32_t, uint32_t>())
        .def("setWeights", &EmbeddingBag::setWeights)
        .def("forward", &EmbeddingBag::forward)
        .def("__call__", &EmbeddingBag::forward);
}
