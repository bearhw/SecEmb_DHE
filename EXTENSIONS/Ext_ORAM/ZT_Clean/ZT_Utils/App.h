/*
 * Copyright (C) 2011-2017 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifndef _APP_H_
#define _APP_H_

#include "Globals.hpp"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h> 
#include <vector>
#include <map>
#include <stdlib.h>
#include <stdarg.h>
#include "LocalStorage.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <cstdint>

#include "Globals.hpp"
#include "CONFIG.h"
#include "Globals_Enclave.hpp"
#include "ORAMTree.hpp"
#include "PathORAM_Enclave.hpp"
#include "CircuitORAM_Enclave.hpp"


#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define MAX_PATH FILENAME_MAX
#define CIRCUIT_ORAM
#define NUMBER_OF_WARMUP_REQUESTS 0
#define ANALYSIS 1
#define MILLION 1E6
#define HASH_LENGTH 32

/*#ifdef DETAILED_MICROBENCHMARKER
  typedef struct detailed_microbenchmark_params{
    uint8_t oram_type;  
    uint8_t recursion_levels;
    uint32_t num_requests;
    bool on;
  }det_mb_params;

  det_mb_params DET_MB_PARAMS;
  det_mb ***MB = NULL; 
  uint32_t req_counter=0;
#endif
*/
//#define NO_CACHING_APP 1
//#define EXITLESS_MODE 1
//#define POSMAP_EXPERIMENT 1


// Global Variables Declarations
uint64_t PATH_SIZE_LIMIT = 1 * 1024 * 1024;
uint32_t aes_key_size = 16;
uint32_t hash_size = 32;	
#define ADDITIONAL_METADATA_SIZE 24
uint32_t oram_id = 0;

uint32_t poram_instance_id=0;
uint32_t coram_instance_id=0;
uint32_t lsoram_instance_id = 0;

//Timing variables
uint32_t recursion_levels_e = 0;

bool resume_experiment = false;
bool inmem_flag = true;

// Storage Backends:
//TODO: Switch to LS for each LSORAM, Path, Circuit
//LocalStorage ls;
//std::map<uint32_t, LocalStorage*> ls_PORAM;

std::vector<PathORAM *> poram_instances;
std::vector<CircuitORAM *> coram_instances;

/*struct oram_request{
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
};*/

struct thread_data{
  struct oram_request *req;
  struct oram_response *resp;
};

struct thread_data td;
struct oram_request req_struct;
struct oram_response resp_struct;	
unsigned char *data_in;
unsigned char *data_out;


uint8_t computeRecursionLevels(uint32_t max_blocks, uint32_t recursion_data_size, uint64_t onchip_posmap_memory_limit);
//void time_report(int report_type, uint8_t level);

uint32_t getNewORAMInstanceID(uint8_t oram_type);
uint8_t createNewORAMInstance(uint32_t instance_id, uint32_t max_blocks, uint32_t data_size, uint32_t stash_size, uint32_t oblivious_flag, uint32_t recursion_data_size, int8_t recursion_levels, uint8_t oram_type, uint8_t pZ);

void accessInterface(uint32_t instance_id, uint8_t oram_type, unsigned char *serialized_request, unsigned char *response, uint32_t serialized_request_size, uint32_t response_size);
//void accessBulkReadInterface(uint32_t instance_id, uint8_t oram_type, uint32_t no_of_requests, unsigned char *serialized_request, unsigned char *response, uint32_t request_size, uint32_t response_size);


void ZT_Close();
uint32_t ZT_New( uint32_t max_blocks, uint32_t data_size, uint32_t stash_size, uint32_t oblivious_flag, uint32_t recursion_data_size, uint32_t oram_type, uint8_t pZ);
// void ZT_Access(uint32_t instance_id, uint8_t oram_type, unsigned char *encrypted_request, unsigned char *encrypted_response, unsigned char *tag_in, unsigned char* tag_out, uint32_t request_size, uint32_t response_size, uint32_t tag_size);
void ZT_Access(uint32_t instance_id, uint8_t oram_type, unsigned char *serialized_request, unsigned char *response, uint32_t request_size, uint32_t response_size);
// void ZT_Bulk_Read(uint32_t instance_id, uint8_t oram_type, uint32_t no_of_requests, unsigned char *encrypted_request, unsigned char *encrypted_response, unsigned char *tag_in, unsigned char* tag_out, uint32_t request_size, uint32_t response_size, uint32_t tag_size);
void ZT_Bulk_Read(uint32_t instance_id, uint8_t oram_type, uint32_t no_of_requests, unsigned char *serialized_request, unsigned char *response, uint32_t request_size, uint32_t response_size);


#endif /* !_APP_H_ */
