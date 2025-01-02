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

/*
Untrusted Application Code for ZeroTrace
Usage : 
printf("./app <N> <No_of_requests> <Stash_size> <Data_block_size> <"resume"/"new"> <"memory"/"hdd"> <"0"/"1" = Non-oblivious/Oblivious> <Recursion_block_size>");
Note : parameters surrounded by quotes should entered in as is without the quotes.

//META-NOTES :
//_e trailed variables are computed/obtained from enclave
//_p trailed variables are obtained from commandline parameters
*/

#include "App.h"

std::map<uint32_t, LocalStorage*> ls_PORAM;
std::map<uint32_t, LocalStorage*> ls_CORAM;

/* Initialize the enclave:
 *   Step 1: try to retrieve the launch token saved by last transaction
 *   Step 2: call sgx_create_enclave to initialize an enclave instance
 *   Step 3: save the launch token if it is updated
 */

uint8_t computeRecursionLevels(uint32_t max_blocks, uint32_t recursion_data_size, uint64_t onchip_posmap_memory_limit) {
  uint8_t recursion_levels = 1;
  uint8_t x;
    
  if(recursion_data_size!=0) {		
    recursion_levels = 1;
    x = recursion_data_size / sizeof(uint32_t);
    uint64_t size_pmap0 = max_blocks * sizeof(uint32_t);
    uint64_t cur_pmap0_blocks = max_blocks;

    while(size_pmap0 > onchip_posmap_memory_limit) {
      cur_pmap0_blocks = (uint64_t) ceil((double)cur_pmap0_blocks/(double)x);
      recursion_levels++;
      size_pmap0 = cur_pmap0_blocks * sizeof(uint32_t);
    }

    #ifdef RECURSION_LEVELS_DEBUG
     printf("IN App: max_blocks = %d\n", max_blocks);
     printf("Recursion Levels : %d\n",recursion_levels);
    #endif
  }
  return recursion_levels;
}

#ifdef DETAILED_MICROBENCHMARKER

  void setMicrobenchmarkerParams(uint8_t oram_type, uint32_t num_requests) {
     DET_MB_PARAMS.oram_type = oram_type;
     DET_MB_PARAMS.num_requests = num_requests;
     DET_MB_PARAMS.on = false;
  }

  void initializeMicrobenchmarker() {
   DET_MB_PARAMS.on = false;
   uint8_t recursion_levels = DET_MB_PARAMS.recursion_levels;
   uint32_t num_reqs = DET_MB_PARAMS.num_requests; 

   
  }

  void initiateMicrobenchmarker(det_mb ***TC_MB) {
    DET_MB_PARAMS.on = true;
    MB=TC_MB;
  }

  uint8_t getRecursionLevels() {
    return(DET_MB_PARAMS.recursion_levels);
  }

#endif

uint32_t getNewORAMInstanceID(uint8_t oram_type){
  if(oram_type==0){
    return poram_instance_id++;
  }
  else if(oram_type==1){
    return coram_instance_id++;
  }
}

uint8_t createNewORAMInstance(uint32_t instance_id, uint32_t max_blocks, uint32_t data_size, uint32_t stash_size, uint32_t oblivious_flag, uint32_t recursion_data_size, int8_t recursion_levels, uint8_t oram_type, uint8_t pZ){

  if(oram_type==0){
    PathORAM *new_poram_instance = new PathORAM();
    int Len1 = poram_instances.size();
    poram_instances.push_back(new_poram_instance);
    int Len2 = poram_instances.size();
    printf("PORAM INSTANCES %d --> %d \n",Len1,Len2);
    
    #ifdef DEBUG_ZT_ENCLAVE
	    printf("In createNewORAMInstance, before Create, recursion_levels = %d\n", recursion_levels);	
    #endif
    
    new_poram_instance->Create(instance_id, oram_type, pZ, max_blocks, data_size, stash_size, oblivious_flag, recursion_data_size, recursion_levels);
    printf("Created PathORAM\n");
    #ifdef DEBUG_ZT_ENCLAVE
	    printf("In createNewORAMInstance, after Create\n");	
    #endif			
    return 0;
  }
  else if(oram_type==1){
    // unreachable("CircuitORAM cannot be created");
    CircuitORAM *new_coram_instance = new CircuitORAM();
    int Len1 = coram_instances.size();
    coram_instances.push_back(new_coram_instance);
    int Len2 = coram_instances.size();
    printf("CORAM INSTANCES %d --> %d \n",Len1,Len2);

    #ifdef DEBUG_ZT_ENCLAVE
    printf("Just before Create\n");
    #endif

    new_coram_instance->Create(instance_id, oram_type, pZ, max_blocks, data_size, stash_size, oblivious_flag, recursion_data_size, recursion_levels);
    printf("Created CircuitORAM\n");
    return 0;
  }
}


void accessInterface(uint32_t instance_id, uint8_t oram_type, unsigned char *serialized_request, unsigned char *response, uint32_t request_size, uint32_t response_size){
  //TODO : Would be nice to remove this dynamic allocation.
  PathORAM *poram_current_instance;
  CircuitORAM *coram_current_instance;
  // printf("In access Interface\n");

  // unsigned char *data_in, *data_out, *request, *request_ptr;
  unsigned char *request_ptr;
  uint32_t id, opType;
  // request = (unsigned char *) malloc (serialized_request_size);
  // data_out = (unsigned char *) malloc (response_size);	

  // printf("Instance1: %d\n", instance_id);

  //Extract Request Id and OpType
  opType = serialized_request[0];
  request_ptr = serialized_request+1;
  memcpy(&id, request_ptr, ID_SIZE_IN_BYTES); 
  data_in = request_ptr+ID_SIZE_IN_BYTES;

  if(oram_type==0){
    poram_current_instance = poram_instances[instance_id];
    // printf("Instance2: %d\n", instance_id);
    poram_current_instance->Access(id, opType, data_in, response);
  }
  else {
    coram_current_instance = coram_instances[instance_id];
    coram_current_instance->Access(id, opType, data_in, response);
  }

}


//Akhi: Work required
// void accessBulkReadInterface(uint32_t instance_id, uint8_t oram_type, uint32_t no_of_requests, unsigned char *serialized_request, unsigned char *response, uint32_t request_size, uint32_t response_size){
//   //TODO : Would be nice to remove this dynamic allocation.
//   PathORAM *poram_current_instance;
//   CircuitORAM *coram_current_instance;
//   unsigned char *data_in, *request, *request_ptr, *response, *response_ptr;
//   uint32_t id;
//   char opType = 'r';

//   uint32_t tdata_size;
//   if(oram_type==0){
//     poram_current_instance = poram_instances[instance_id];
//     tdata_size = poram_current_instance->data_size;
//   }	
//   else{
//     coram_current_instance = coram_instances[instance_id];
//     tdata_size = coram_current_instance->data_size;
//   }
  
//   request = (unsigned char *) malloc (request_size);
//   response = (unsigned char *) malloc (response_size);	
//   data_in = (unsigned char *) malloc(tdata_size);

//   /*sgx_status_t status = SGX_SUCCESS;
//   status = sgx_rijndael128GCM_decrypt((const sgx_aes_gcm_128bit_key_t *) SHARED_AES_KEY, (const uint8_t *) encrypted_request,
// 			      encrypted_request_size, (uint8_t *) request, (const uint8_t *) HARDCODED_IV, IV_LENGTH,
// 			      NULL, 0, (const sgx_aes_gcm_128bit_tag_t*) tag_in);
//   */
//   /*
//   if(status == SGX_SUCCESS)
// 	  printf("Decrypt returned Success flag\n");
//   else{
// 	  if(status == SGX_ERROR_INVALID_PARAMETER)
// 		  printf("Decrypt returned SGX_ERROR_INVALID_PARAMETER Failure flag\n");		
// 	  else
// 		  printf("Decrypt returned another Failure flag\n");
//   }
//   */

//   request_ptr = request;
//   response_ptr = response;

//   for(int l=0; l<no_of_requests; l++){			
//     //Extract Request Ids
//     memcpy(&id, request_ptr, ID_SIZE_IN_BYTES);
//     request_ptr+=ID_SIZE_IN_BYTES; 

//     //TODO: Fix Instances issue.
//     if(oram_type==0)
// 	    poram_current_instance->Access(id, opType, data_in, response_ptr);
//     else
// 	    coram_current_instance->Access(id, opType, data_in, response_ptr);
//     response_ptr+=(tdata_size);
//   }

//   //Encrypt Response
//   /*
//   status = sgx_rijndael128GCM_encrypt((const sgx_aes_gcm_128bit_key_t *) SHARED_AES_KEY, response, response_size,
// 				  (uint8_t *) encrypted_response, (const uint8_t *) HARDCODED_IV, IV_LENGTH, NULL, 0,
// 				  (sgx_aes_gcm_128bit_tag_t *) tag_out);
//   */
//   /*
//   if(status == SGX_SUCCESS)
// 	  printf("Encrypt returned Success flag\n");
//   else{
// 	  if(status == SGX_ERROR_INVALID_PARAMETER)
// 		  printf("Encrypt returned SGX_ERROR_INVALID_PARAMETER Failure flag\n");		
// 	  else
// 		  printf("Encrypt returned another Failure flag\n");
//   }
//   */

//   free(request);
//   free(response);
//   free(data_in);

// }


uint32_t ZT_New( uint32_t max_blocks, uint32_t data_size, uint32_t stash_size, uint32_t oblivious_flag, uint32_t recursion_data_size, uint32_t oram_type, uint8_t pZ) {
  uint32_t sgx_return = 0;
  int8_t rt;
  uint8_t urt;
  uint32_t instance_id;
  int8_t recursion_levels;
  LocalStorage *ls_oram = new LocalStorage();    

  // RecursionLevels is really number of levels of ORAM
  // So if no recursion, recursion_levels = 1 
  recursion_levels = computeRecursionLevels(max_blocks, recursion_data_size, MEM_POSMAP_LIMIT);
  printf("APP.cpp : ComputedRecursionLevels = %d\n", recursion_levels);
    
  uint32_t D = (uint32_t) ceil(log((double)max_blocks/pZ)/log((double)2));
  printf("App.cpp: params for LS : \n \(%d, %d, %d, %d, %d, %d, %d, %d)\n",
         max_blocks,D,pZ,stash_size,data_size + ADDITIONAL_METADATA_SIZE,inmem_flag, recursion_data_size + ADDITIONAL_METADATA_SIZE, recursion_levels);
  
  // LocalStorage Module, just works with recursion_levels 0 to recursion_levels 
  // And functions without treating recursive and non-recursive backends differently
  // Hence recursion_levels passed = recursion_levels,

  ls_oram->setParams(max_blocks,D,pZ,stash_size,data_size + ADDITIONAL_METADATA_SIZE,inmem_flag, recursion_data_size + ADDITIONAL_METADATA_SIZE, recursion_levels);

  #ifdef DETAILED_MICROBENCHMARKER  
   printf("DET_MB_PARAMS.recursion_levels = %d\n", recursion_levels);
   DET_MB_PARAMS.recursion_levels = recursion_levels; 
   // Spawn required variables for microbenchmarker
   initializeMicrobenchmarker();
   // Client flags the DET_MB_PARAMS, by setting a bool ON to start
   // the detailed microbenchmarking 
  #endif

  instance_id = getNewORAMInstanceID(oram_type);
  printf("INSTANCE_ID returned = %d\n", instance_id);  

  if(oram_type==0){
    ls_PORAM.insert(std::make_pair(instance_id, ls_oram));
    printf("Inserted instance_id = %d into ls_PORAM\n", instance_id);
  }
  else if(oram_type==1) {
    // unreachable("Circuit ORAM not implemented");
    ls_CORAM.insert(std::make_pair(instance_id, ls_oram));
    printf("Inserted instance_id = %d into ls_CORAM\n", instance_id);
  }

  uint8_t ret;
  sgx_return = createNewORAMInstance(instance_id, max_blocks, data_size, stash_size, oblivious_flag, recursion_data_size, recursion_levels, oram_type, pZ);


  #ifdef DEBUG_PRINT
      printf("initialize_oram Successful\n");
  #endif
  return (instance_id);
}

//Request and response sizes can probably be removed
void ZT_Access(uint32_t instance_id, uint8_t oram_type, unsigned char *serialized_request, unsigned char *response, uint32_t request_size, uint32_t response_size) {
  // printf("In ZT_Access\n");
  #ifdef DETAILED_MICROBENCHMARKER
    static struct timespec start, end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  #endif

    accessInterface(instance_id, oram_type, serialized_request, response, request_size, response_size);
  
  #ifdef DETAILED_MICROBENCHMARKER
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    double total_time = timetaken(&start, &end);

    if(DET_MB_PARAMS.on == true) {
      MB[req_counter][0]->total_time=total_time;
      req_counter++; 
        if(DET_MB_PARAMS.num_requests==req_counter) { 
          req_counter=0;
          DET_MB_PARAMS.on = false;
        }
    }
  #endif
 
}

void ZT_Bulk_Read(uint32_t instance_id, uint8_t oram_type, uint32_t no_of_requests, unsigned char *serialized_request, unsigned char *response, uint32_t request_size, uint32_t response_size) {
  unreachable("Bulk Read not implemented");
    // accessBulkReadInterface(instance_id, oram_type, no_of_requests, serialized_request, response, request_size, response_size);
}


