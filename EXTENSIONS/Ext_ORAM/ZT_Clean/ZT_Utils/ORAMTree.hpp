/*
*    ZeroTrace: Oblivious Memory Primitives from Intel SGX 
*    Copyright (C) 2018  Sajin (sshsshy)
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, version 3 of the License.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/  

#ifndef __ZT_ORAMTREE__
  #include <string.h>
  #include "Globals_Enclave.hpp"
  #include "LocalStorage.hpp"
  #include "Bucket.hpp" 
  #include "Stash.hpp"
  #include "openssl/aes.h"
  #include "openssl/evp.h"


  class ORAMTree {
    public:

    //Basic Params
    uint8_t Z;
    uint32_t data_size;
    uint32_t stash_size;
    bool oblivious_flag;
    uint8_t recursion_levels;
    uint32_t recursion_data_size;

    //Params to tie ORAM to corresponding LS backend
    uint32_t instance_id;
    uint8_t oram_type;

    //Buffers
    unsigned char* encrypted_path;
    unsigned char* decrypted_path;
    unsigned char* downloaded_path;
    unsigned char* fetched_path_array;
    unsigned char* serialized_result_block;
			
    //Computed Params
    uint32_t x;
    uint64_t gN;
    uint32_t treeSize;

    //PositionMap
    uint32_t* posmap;

    //Stash components
    Stash *recursive_stash;

    // Parameters for recursive ORAMs (these are per arrays for corresponding parameters for each level of recursion)
    uint64_t *max_blocks_level; //The total capacity of blocks in a level
    uint64_t *real_max_blocks_level; //The real blocks used out of that total capacity
    //This is an artifact of recursion, in non-recursive they are the same.
    uint64_t *N_level; //For non-recursive, N = N_level[0]
    uint32_t *D_level; //For non-recursive, D = D_level[0]
    // sgx_sha256_hash_t* merkle_root_hash_level; 
    //For non-recursive, merkle_root = merkle_root_hash_level[0] 

    //Key components		
    unsigned char *aes_key;

    //Debug Functions
    void print_stash_count(uint32_t level, uint32_t nlevel);
    void print_pmap0();
    void showPath(unsigned char *decrypted_path, uint8_t Z, uint32_t d, uint32_t data_size);
    void showPath_reverse(unsigned char *decrypted_path, uint8_t Z, uint32_t d, uint32_t data_size, uint32_t leaf_bucket_id);
    void showPath_BlockIDLabel(unsigned char *decrypted_path, uint8_t Z, uint32_t d, uint32_t data_size, uint32_t bucket_id_of_leaf);

    //Initialize/Build Functions
    //Helper function for BuildTree Recursive
    uint32_t* BuildTreeLevel(uint8_t level, uint32_t* prev_posmap); 
    // For non-recursive, simply invoke BuildTreeRecursive(0,NULL)
    void BuildTreeRecursive(uint8_t level, uint32_t *prev_pmap);
    void Initialize();
    void SetParams(uint32_t instance_id, uint8_t oram_type, uint8_t pZ, uint32_t pmax_blocks, uint32_t pdata_size, uint32_t pstash_size, uint32_t poblivious_flag, uint32_t precursion_data_size, uint8_t precursion_levels);
    void SampleKey();

    //Constructor & Destructor
    ORAMTree();
    ~ORAMTree();

    void aes_dec_serialized(unsigned char* encrypted_block, uint32_t data_size, unsigned char *decrypted_block, unsigned char* aes_key);
    void aes_enc_serialized(unsigned char* decrypted_block, uint32_t data_size, unsigned char *encrypted_block, unsigned char* aes_key);
    void decryptPath(unsigned char* path_array, unsigned char *decrypted_path_array, uint32_t num_of_blocks_on_path, uint32_t data_size);
    void encryptPath(unsigned char* path_array, unsigned char *encrypted_path_array, uint32_t num_of_blocks_on_path, uint32_t data_size);

    unsigned char* downloadPath(uint32_t leaf, uint8_t level);

    void printPath(uint32_t leaf, uint8_t level);
    void printPath_buffer(unsigned char* path, uint32_t length, uint32_t data_size);
    void print_last_level(uint8_t level, uint32_t start, uint32_t end);
    void xordata64(unsigned char* data, unsigned char* nonce, uint32_t data_size, bool blockdata_flag=0);

    void uploadPath(uint32_t leaf, unsigned char *path, uint64_t path_size, uint8_t level);
    void refreshnonce(unsigned char* dec_path, uint32_t level);
    void xornonce(unsigned char* before_block, uint32_t data_size, unsigned char *after_block, bool flag);
    //Corresponding OCALL is ::
    //uploadPath(&rt, encrypted_path, path_size, leaf + nlevel, new_path_hash, new_path_hash_size, level, D_level);

    //Access Functions
    void PushBlocksFromPathIntoStash(unsigned char* decrypted_path_ptr, uint8_t level, uint32_t data_size, uint32_t block_size, uint32_t id, uint32_t position_in_id, uint32_t leaf, uint32_t *nextLeaf, uint32_t newleaf, uint32_t sampledLeaf, int32_t newleaf_nextlevel);
    uint32_t access_oram_level(char opType, uint32_t leaf, uint32_t id, uint32_t position_in_id, uint8_t level, uint32_t newleaf,uint32_t newleaf_nextleaf, unsigned char *data_in,  								unsigned char *data_out);		


    //Misc                
    //uint32_t savePosmap(unsigned char *posmap_serialized, uint32_t posmap_size); 
    void OAssignNewLabelToBlock(uint32_t id, uint32_t position_in_id, uint8_t level, uint32_t newleaf, uint32_t newleaf_nextlevel, uint32_t * nextLeaf);
  };

    void oarray_search(uint32_t *array, uint32_t loc, uint32_t *leaf, uint32_t newLabel,uint32_t N_level);

    uint8_t uploadPath_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* path_array, uint32_t path_size, uint32_t leaf_label, uint8_t level, uint32_t D_level);
    uint8_t uploadBucket_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* serialized_bucket, uint32_t bucket_size, uint32_t label, uint32_t size_for_level, uint8_t recursion_level);
    uint8_t downloadPath_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* path_array, uint32_t path_size, uint32_t leaf_label, uint8_t level, uint32_t D_level);
    uint8_t downloadBucket_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* serialized_bucket, uint32_t bucket_size, uint32_t label, unsigned char* hash, uint32_t hashsize, uint32_t size_for_level, uint8_t recursion_level);

  #define __ZT_ORAMTREE__
#endif
