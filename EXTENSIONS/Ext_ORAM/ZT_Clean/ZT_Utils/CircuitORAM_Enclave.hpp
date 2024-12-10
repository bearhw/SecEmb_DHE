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

#ifndef __ZT_CIRCUITORAM__
  #include <stdint.h>
  #include <string.h>
  #include "Globals_Enclave.hpp"
  #include "Block.hpp"
  #include "Bucket.hpp"
  #include "ORAMTree.hpp"

  // #include <bsd/stdlib.h>
  #include <stdlib.h>
  #include "ORAM_Interface.hpp"
  #include "oasm_lib.h" 
  #include "time.h"
  #include <random>

  // double timetaken(timespec *start, timespec *end);
  // void time_report(int report_type, uint8_t level);


  class CircuitORAM: public ORAMTree, public ORAM_Interface 
  {
    public:
    //Specific to CircuitORAM
    uint32_t *deepest;
    uint32_t *target;
    int32_t *deepest_position;
    int32_t *target_position;
    unsigned char *serialized_block_hold;
    unsigned char *serialized_block_write;
    //For deterministic reverse lexicographic eviction
    uint32_t *access_counter;

    // uint32_t* newleaf_arr;
    // uint32_t newleaf_ctr;

    CircuitORAM(){};

    uint32_t* GenerateRandomSequence(uint32_t length, uint32_t start, uint32_t num_blocks);

    void Initialize(uint32_t instance_id, uint8_t oram_type, uint8_t pZ, uint32_t pmax_blocks, uint32_t pdata_size, uint32_t pstash_size, uint32_t poblivious_flag, uint32_t precursion_data_size, uint8_t precursion_levels);
    
    uint32_t CircuitORAM_Access(char opType, uint32_t id, uint32_t position_in_id, uint32_t leaf, uint32_t newleaf, uint32_t newleaf_nextlevel, unsigned char* decrypted_path, 
                                uint32_t level, unsigned char* data_in, unsigned char *data_out);
    //void Access_temp(uint32_t id, char opType, unsigned char* data_in, unsigned char* data_out);	
    uint32_t access(uint32_t id, int32_t position_in_id, char opType, uint8_t level, unsigned char* data_in, unsigned char* data_out, uint32_t *prev_sampled_leaf);			
    uint32_t access_oram_level(char opType, uint32_t leaf, uint32_t id, uint32_t position_in_id, uint32_t level, uint32_t newleaf,uint32_t newleaf_nextleaf, unsigned char *data_in,  unsigned char *data_out);
    

    void CircuitORAM_FetchBlock(uint32_t *return_value, uint32_t leaf, uint32_t newleaf, char opType,
        uint32_t id, uint32_t position_in_id, uint32_t newleaf_nextlevel, uint32_t level, unsigned char *data_in, unsigned char *data_out);
    
    void CircuitORAM_RebuildPath(unsigned char* decrypted_path_ptr, uint32_t data_size, uint32_t block_size, uint32_t leaf, uint32_t level);

    void EvictionRoutine(uint32_t leaf, uint32_t level);

    //Additional CircuitORAM functions
    uint32_t* prepare_target(uint32_t leaf, unsigned char *serialized_path, uint32_t block_size, uint32_t level, uint32_t * deepest, int32_t *target_position);
    uint32_t* prepare_deepest(uint32_t leaf, unsigned char *serialized_path, uint32_t block_size, uint32_t level, int32_t *deepest_position);	
    void EvictOnceFast(uint32_t *deepest, uint32_t *target, int32_t* deepest_position, int32_t* target_position , unsigned char * serialized_path, uint32_t level, uint32_t leaf);
   
    //Virtual functions, inherited from ORAMTree class 
    void Create(uint32_t instance_id, uint8_t oram_type, uint8_t pZ, uint32_t pmax_blocks, uint32_t pdata_size, uint32_t pstash_size, uint32_t poblivious_flag, uint32_t precursion_data_size, uint8_t precursion_levels) override;	
    void Access(uint32_t id, char opType, unsigned char* data_in, unsigned char* data_out) override;

  
  };

  #define __ZT_CIRCUITORAM__
#endif
