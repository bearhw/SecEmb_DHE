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

#ifndef __ZT_CONFIG__ 
 
  //BACKEND choices 0 = Memory, 1 = HDD
  #define BACKEND 0

  // The PRM memory limit for Position Map of a ZeroTrace ORAM Instance in bytes.
  // (The recursion levels of the ORAM is paramterized by this value)
  #define MEM_POSMAP_LIMIT 1024*1024*1024

  // If turned on, the client (TestCorrectness) will have a detailed microbenchmark
  // of time taken in each of part of the ORAM Access operation  
  #define DETAILED_MICROBENCHMARKER 1

  #define REVERSE_LEXICOGRAPHIC_EVICTION 1

  #define __ZT_CONFIG__ 
#endif 

#ifndef __ZT_CONFIG_FLAGS__

  #define CHECK_CORRECTNESS 1
  // #define ENCRYPTION_ON 1
  // #define ENC_XOR 1
  #define RAND_DATA 1
  #define INLINE_ASM 1
  // #define PROTECT_POSMAP 1
  // #define PROTECT_STASH 1
  // #define PROTECT_STASH_DEBUG 1
  #define CACHE_LINE_SIZE 64
  #define NEW_OMOVE2 1

  #define SHOW_TIMING_RESULTS 1
  // VERBOSE Adds additional information of intermediate steps status
  //#define VERBOSE 1
  //#define EXITLESS_MODE 1 
  #define PASSIVE_ADVERSARY 1

  //Flags added from VT
  //#define SMALL_TEST
   
  // Storage Flags 
    //#define DEBUG_LS 1
    //#define LS_DEBUG_INTEGRITY 1

  // Client Flags
    // #define PRINT_REQ_DETAILS 1
    // #define RESULTS_DEBUG2 1


  //Enclave-specific Flags
    //#define DEBUG_ZT_ENCLAVE 1
    //#define SET_PARAMETERS_DEBUG 1
    //#define BUILDTREE_DEBUG 1
    //#define ACCESS_DEBUG 1
    //#define DEBUG_INTEGRITY 1 
 
    //#define BUILDTREE_VERIFICATION_DEBUG 1
    // #define SHOW_STASH_COUNT_DEBUG 1 
    //#define SHOW_STASH_CONTENTS 1
    //#define DEBUG_EFO 1
    // #define RESULTS_DEBUG 1
    //#define PAO_DEBUG 1

    //#define PATHORAM_ACCESS_REBUILD_DEBUG 1
    #define STASH_OVERFLOW_DEBUG 1 
    //#define ACCESS_CORAM_DEBUG 1
    // #define ACCESS_CORAM_META_DEBUG 1
    //#define ACCESS_CORAM_DEBUG3 1
    //#define ACCCES_DEBUG_EXITLESS 1
    //#define ACCESS_DEBUG_REBUILD 1 

    //#define DEBUG_PRINT 1
    //#define RECURSION_LEVELS_DEBUG 1


  //Variable #defines
  //define FLAGS :
   #define TIME_PERFORMANCE 1


  enum PATHORAM_TIMER {PO_POSMAP_START, PO_POSMAP_END, PO_DOWNLOAD_PATH_START, PO_DOWNLOAD_PATH_END,
   PO_FETCH_BLOCK_START, PO_FETCH_BLOCK_END, PO_EVICTION_START, PO_EVICTION_END, 
   PO_UPLOAD_PATH_START, PO_UPLOAD_PATH_END};
 
  enum CIRCUITORAM_TIMER {CO_POSMAP_START, CO_POSMAP_END, CO_FETCH_BLOCK_START, CO_FETCH_BLOCK_END,
   CO_DOWNLOAD_PATH_START, CO_DOWNLOAD_PATH_END, CO_UPLOAD_PATH_START, CO_UPLOAD_PATH_END, 
   CO_EVICTION_START, CO_EVICTION_END}; 

  #define __ZT_CONFIG_FLAGS__ 
#endif
