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

#ifndef __ZT_GLOBALS_ENCLAVE__
  #define __ZT_GLOBALS_ENCLAVE__
  #include "Globals.hpp"
  #include "CONFIG.h"
  #include <string.h>
  #include "ORAM_Interface.hpp"
  #include <stdarg.h>
  #include <stdio.h>      /* vsnprintf */
  //#include "Enclave_t.h"  /* print_string */
  #include <stdlib.h>
  #include <stdio.h>
  #include <stdint.h>
  #include <math.h>
  #include <stdlib.h>
  //#include <sgx_tcrypto.h>
  //#include "sgx_trts.h"
  #include <openssl/ec.h>
  #include <assert.h>
  #include <map>
  #include "LocalStorage.hpp"

  extern LocalStorage ls;
  extern std::map<uint32_t, LocalStorage*> ls_PORAM;
  extern std::map<uint32_t, LocalStorage*> ls_CORAM;


  struct oram_request{
    uint32_t *id;
    uint32_t *level;
    uint32_t *d_lev;
    bool *recursion;
    bool *block;
  };

  struct oram_response{
    unsigned char *path;
    unsigned char *path_hash;
    unsigned char *new_path;
    unsigned char *new_path_hash;
  };

  struct nodev2{
    unsigned char *serialized_block;
    struct nodev2 *next;
  };

  static inline uint64_t cmov(int pred, uint64_t val1, uint64_t val2)
{
    uint64_t result;
    __asm__(
        "mov %2, %0\n\t"
        "test %1, %1\n\t"
        "cmovz %3, %0\n\t"
        : [output] "=&r"(result) // & means early clobber
        : [input] "r"(pred), "r"(val1), "r"(val2)
        : "cc");
    return result;
}

static inline void oaccess_stash_data(unsigned char* data_dest, unsigned char* data_src, unsigned char* nonce ,uint32_t data_size, bool flag){
	  uint64_t *src = (uint64_t *)data_src; // moves 8-bytes at a time, utilizing 64-bit registers
    uint64_t *dst = (uint64_t *)data_dest;
    uint64_t *nonce64 = (uint64_t *)nonce;

    bool rd_nonce_idx = 1;
    uint64_t res;
    uint64_t size = 0;
    int i = 0;

    while(size<data_size){
        dst[i] = cmov(flag, src[i]^nonce64[1], dst[i]);
        i++;
        size = size+8;
        if(size>=data_size){
			    break;
        }
        dst[i] = cmov(flag, src[i]^nonce64[0], dst[i]);
        i++;
        size = size+8;
    }

}

static inline void o_refreshnonce(unsigned char* serialized_block, unsigned char* nonce, uint32_t data_size, bool flag){
  // unsigned char* diffnonce = (unsigned char*)malloc(NONCE_LENGTH);
  uint32_t *nonceptr32 = (uint32_t*)nonce;
  unsigned char* xorpath_ptr = serialized_block;
  
  uint64_t* nonce64 = (uint64_t*)nonce;
  uint64_t* xorpath64 = (uint64_t*)xorpath_ptr;

  uint32_t data_to_encrypt = data_size + ADDITIONAL_METADATA_SIZE;

  uint32_t size = 0;
  int j = 0;

  while(size<NONCE_LENGTH){
    xorpath64[j] = cmov(flag, xorpath64[j]^nonce64[0], xorpath64[j]);
    size = size + 8;
    j++;
    if(size>=data_size){
      break;
    }
    xorpath64[j] = cmov(flag, xorpath64[j]^nonce64[1], xorpath64[j]);
    size = size + 8;
    j++;
  }

  uint32_t *idptr = getIdPtr(xorpath_ptr);
  *idptr = cmov(flag, *idptr^*nonceptr32, *idptr);

  uint32_t *label = getTreeLabelPtr(xorpath_ptr);
  *label = cmov(flag, *label^(*((uint32_t*)(nonce+4))), *label);

  j=0;
  size=0;
  uint64_t *dataptr64 = (uint64_t*)(getDataPtr(xorpath_ptr));

  while(size<data_size){
    dataptr64[j] = cmov(flag, dataptr64[j]^nonce64[1], dataptr64[j]);
    size = size + 8;
    j++;
    if(size>=data_size){
      break;
    }
    dataptr64[j] = cmov(flag, dataptr64[j]^nonce64[0], dataptr64[j]);
    size = size + 8;
    j++;
  }
  
    
}

static inline void omove_buffer_NEW(unsigned char *dest, unsigned char *source, uint32_t buffersize, uint32_t flag)
{
    uint64_t *src = (uint64_t *)source; // moves 8-bytes at a time, utilizing 64-bit registers
    uint64_t *dst = (uint64_t *)dest;

    uint32_t count = buffersize / 8;
    uint64_t res;
    for (uint32_t i = 0; i < count; i++)
    {
        uint64_t orig = dst[i];
        uint64_t new_ = src[i];
        res = cmov(flag, new_, orig); // only thing written in assembly
        dst[i] = res;
    }
}

//Faster by 9.8% for circuit, slower by 5.8% for path
static inline void omove_NEW(uint32_t i, uint32_t *item, uint32_t loc, uint32_t *leaf, uint32_t newLabel)
{
  bool flag = (i == loc);
  *leaf = cmov(flag, *item, *leaf);
  *item = cmov(flag, newLabel, *item);
}

//This function obliviously XORs the posmap's 64 bit nonce with all of posmap, taken 64 bits at a time.
static inline void oarray_search_NEW(uint32_t *array, uint32_t loc, uint32_t *leaf, uint32_t newLabel,uint32_t posmap_size)
{
  bool flag, divflag;
  uint64_t randval = rand();
  uint64_t templeaf;
  uint64_t *array64 = (uint64_t*)array;
  uint32_t loopsize = (posmap_size + (posmap_size&1))/2;
  uint64_t *nonce = &array64[loopsize];


  for(int i = 0; i < loopsize; i++){
    flag = (i==loc/2);
    divflag = loc&1;

    uint32_t temp_leaf = cmov(divflag, (uint32_t)((array64[i]^*nonce)>>32), (uint32_t) ((array64[i]^*nonce)));
    *leaf = cmov(flag, temp_leaf, *leaf);

    uint64_t newnewlabel = ((uint64_t)(newLabel)) << 32;
    uint64_t templeaf_odd = ((array64[i]^*nonce) & 0x00000000ffffffff) | newnewlabel;
    uint64_t templeaf_even = ((array64[i]^*nonce) & 0xffffffff00000000) | ((uint64_t) newLabel);

    uint64_t arrtemp = cmov(divflag, templeaf_odd^*nonce, templeaf_even^*nonce);
    array64[i] = cmov(flag, arrtemp^randval, array64[i]^randval);

  }
  *nonce = *nonce^randval;
}

//Slows down CircuitORAM by ~8.98%
static inline void pt_settarget_NEW(uint32_t *target, int32_t* dest, int32_t* src, uint32_t flag)
{
    *target = cmov(flag, *dest, *target);
    *src = cmov(flag, -1, *src);
    *dest = cmov(flag, -1, *dest);
}

//Almost no difference in performance
static inline void pd_setdeepest_NEW(uint32_t *deepest, int32_t src, uint32_t flag)
{
  *deepest = cmov(flag, src, *deepest);
}

//Very slightly faster for circuit
static inline void oassign_newlabel_NEW(uint32_t *ptr_to_label, uint32_t newLabel, bool flag)
{
  *ptr_to_label = cmov(flag, newLabel, *ptr_to_label);
}

//Maybe slightly faster
static inline void oset_goal_source_NEW(uint32_t level_in_path, uint32_t local_deepest_in_path, uint32_t flag, int32_t *src, int32_t *goal)
{
  *src = cmov(flag, level_in_path, *src);
  *goal = cmov(flag, local_deepest_in_path, *goal);
}

//Maybe slightly faster
static inline void oset_hold_dest_NEW(int32_t* hold, int32_t* dest, uint32_t* wflag, uint32_t flag)
{

  *hold = cmov(flag, -1, *hold);
  *dest = cmov(flag, -1, *dest);
  *wflag = cmov(flag, 1, *wflag);

}

//Slower by ~8% for circuit
static inline void oset_block_as_dummy_NEW(unsigned char* serialized_block, uint32_t gN, uint32_t flag)
{
  uint32_t *block_id = getIdPtr(serialized_block);
  uint32_t tempgN = gN;
  #ifdef PROTECT_STASH
    uint32_t *nonceptr = (uint32_t*)serialized_block;
    tempgN = (*nonceptr) ^ gN;
  #endif
  *block_id = cmov(flag, tempgN, *block_id);
}

static inline void pt_set_target_position_NEW(int32_t *target_position, uint32_t pos_in_bucket, uint32_t* target_position_set_flag, uint32_t flag)
{
  *target_position = cmov(flag, pos_in_bucket, *target_position);
  *target_position_set_flag = cmov(flag, 1, *target_position_set_flag);
}

static inline void pt_set_src_dest_NEW(int32_t *src, int32_t *dest, int32_t deepest_of_i_1, uint32_t i, uint32_t flag_pt)
{
  *src = cmov(flag_pt, deepest_of_i_1, *src);
  *dest = cmov(flag_pt, i, *dest);
}

static inline void oset_value_NEW(uint32_t *dest, uint32_t value, uint32_t flag)
{
  *dest = cmov(flag, value, *dest);
}

static inline void oincrement_value_NEW(uint32_t *dest, uint32_t flag)
{
  uint32_t add_val = *dest + 1;
  *dest = cmov(flag, add_val, *dest);
}


#endif
