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

#ifndef __ZT_utils__

  #include "../ZT_Utils/Globals.hpp"
  #include "../ZT_Utils/CONFIG.h"
  // #include "../ZT_Utils/CONFIG_FLAGS.h"
  #include <stdint.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <iostream>
  #include <string.h>
  #include <random>
  #include <math.h>
  #include "ZT.hpp"
  #include <openssl/ec.h>
  #include <openssl/ecdh.h>
  #include <openssl/ecdsa.h>
  #include <openssl/conf.h>
  #include <openssl/evp.h>
  #include <openssl/err.h>
  #include <openssl/obj_mac.h>

  class RandomRequestSource
  {
  public:
    RandomRequestSource(){};
    // Generates a random sequence of <length> elements, where each element in 
    // [0, <max_capacity-1>]
    uint32_t* GenerateRandomSequence(uint32_t length, uint32_t max_capacity);

    // Generates RandomPermutation of length elements, such that all <length> 
    // elements are unique and from [0, <length>-1]
    uint32_t* GenerateRandomPermutation(uint32_t length);
  };

  //Time Functions
    double time_taken(timespec *start, timespec *end);

  //Buffer-handling/Serialize Functions
    void serializeRequest(uint32_t request_id, char op_type, unsigned char *data,
         uint32_t data_size, unsigned char* serialized_request);

    void prepareDataBlock(unsigned char *datablock, uint32_t index, uint32_t data_size);
    int checkFetchedDataBlock(unsigned char *datablock, uint32_t index, uint32_t data_size);

    //Timing and other functions
    double compute_avg(double *elements, uint32_t num_elements);
    double compute_stddev(double *elements, uint32_t num_elements);

    // inline void unreachable(std::string msg);

  #define __ZT_utils__
#endif
