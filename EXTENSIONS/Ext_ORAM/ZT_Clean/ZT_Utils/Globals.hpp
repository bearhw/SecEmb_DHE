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

#ifndef __GLOBALS__
  
  #include <stdint.h>
  #include <string>
  #include <iostream>
  //Global Flags

  #include "CONFIG.h"

  // Global Declarations

  #define ADDITIONAL_METADATA_SIZE 24
  #define HASH_LENGTH 32
  #define NONCE_LENGTH 16
  #define KEY_LENGTH 16
  #define MILLION 1E6
  #define IV_LENGTH 12
  #define ID_SIZE_IN_BYTES 4
  #define TAG_SIZE 16
  #define CLOCKS_PER_MS (CLOCKS_PER_SEC/1000)
  #define AES_GCM_BLOCK_SIZE_IN_BYTES 16
  #define PRIME256V1_KEY_SIZE 32



  //Other Global variables

  //Hard-coded Enclave Signing key
  //This key would ideally be sampled and signed in the Remote attestation phase with a client
  //Currently we use a static hard coded ECDSA key for it.

  #ifndef TRUE
  # define TRUE 1
  #endif

  #ifndef FALSE
  # define FALSE 0
  #endif

  typedef struct{
    unsigned char *key;
    unsigned char *value;
  }tuple;

  typedef struct detailed_microbenchmarks{
   double posmap_time;
   double download_path_time;
   double fetch_block_time;
   double eviction_time;
   double upload_path_time; 
   double total_time; 
  }det_mb;

  //Inline Functions
  inline uint32_t iBitsPrefix(uint32_t n, uint32_t w, uint32_t i){
    return (~((1<<(w-i)) - 1)) & n;
  }

  inline uint32_t ShiftBy(uint32_t n, uint32_t w) {
    return(n>>w);
  }

  inline uint32_t noOfBitsIn(uint32_t local_deepest){
    uint32_t count = 0;
    while(local_deepest!=0){
      local_deepest = local_deepest >>1;
      count++;
    }
    return count;
  }

  inline bool isBlockDummy(unsigned char *serialized_block, uint64_t gN){
    #ifdef PROTECT_STASH
      uint32_t *nonceptr = (uint32_t*)(serialized_block);
      bool dummy_flag = ((*((uint32_t*)(serialized_block+16)))^(*nonceptr))==gN;
      // bool dummy_flag2 = ((*((uint32_t*)(serialized_block+16)))^(*nonceptr))==0;
      // dummy_flag = dummy_flag & dummy_flag2;
    #else
      bool dummy_flag = *((uint32_t*)(serialized_block+16))==gN;
    #endif

    return dummy_flag;
  }

  inline void setBlockasDummy(unsigned char *serialized_block, uint64_t gN){
    uint32_t *idptr = ((uint32_t*)(serialized_block+16));
    #ifdef PROTECT_STASH
      uint32_t *nonceptr = (uint32_t*)(serialized_block);
      *idptr = *idptr^(uint32_t)gN;
    #else
      *idptr = (uint32_t)gN;
    #endif
  }

  inline unsigned char* getNoncePtr(unsigned char *serialized_block){
    return (unsigned char*)serialized_block;
  }

  inline uint32_t getId(unsigned char *serialized_block){
    #ifdef PROTECT_STASH
      uint32_t* nonceptr = (uint32_t*)(serialized_block);
      uint32_t encrypted_id = *((uint32_t*)(serialized_block+16));
      uint32_t id = encrypted_id^(*nonceptr);
    #else
      uint32_t id = *((uint32_t*)(serialized_block+16));
    #endif

    return id;
  }

  inline uint32_t* getIdPtr(unsigned char *serialized_block){
    uint32_t *id = ((uint32_t*)(serialized_block+16));
    return id;
  }

  inline void setId(unsigned char *serialized_block, uint32_t new_id){
    #ifdef PROTECT_STASH
      uint32_t* nonceptr = (uint32_t*)(serialized_block);
      *((uint32_t*)(serialized_block+16)) = new_id^(*nonceptr);
    #else
      *((uint32_t*)(serialized_block+16)) = new_id;
    #endif

  }

  inline uint32_t getTreeLabel(unsigned char *serialized_block){
    #ifdef PROTECT_STASH
      uint32_t *nonceptr = ((uint32_t*)(serialized_block+4));
      uint32_t *labelptr = ((uint32_t*)(serialized_block+20));
      uint32_t treeLabel = (*labelptr)^(*nonceptr);
    #else
      uint32_t treeLabel = *((uint32_t*)(serialized_block+20));
    #endif

    return treeLabel;
  }

  inline uint32_t* getTreeLabelPtr(unsigned char *serialized_block){
    uint32_t *labelptr = ((uint32_t*)(serialized_block+20));
    return labelptr;
  }

  inline void setTreeLabel(unsigned char *serialized_block, uint32_t new_treelabel){
    #ifdef PROTECT_STASH
      uint32_t* nonceptr = (uint32_t*)(serialized_block+4);
      *((uint32_t*)(serialized_block+20)) = new_treelabel^(*nonceptr);
    #else
      *((uint32_t*)(serialized_block+20)) = new_treelabel;
    #endif

  }

  inline unsigned char* getDataPtr(unsigned char* decrypted_path_ptr){
    return (unsigned char*) (decrypted_path_ptr+24);
  }

  inline uint32_t printData(unsigned char* dataptr, unsigned char* nonceptr){
    uint32_t *data32 = (uint32_t*)dataptr;
    uint32_t *nonce32 = (uint32_t*)(nonceptr + 8);

    return ((*data32) ^ (*nonce32));

  }

  inline void unreachable(std::string msg = "<No Message>") {
    std::cerr << "Unreachable code reached. Message: " << msg << std::endl;
    std::abort();
  }


  #define __GLOBALS__
#endif

