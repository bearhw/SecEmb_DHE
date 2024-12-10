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

#include "utils.hpp"

uint32_t* RandomRequestSource::GenerateRandomSequence(uint32_t length, uint32_t max_capacity) {
	uint32_t* requestsource = (uint32_t *) malloc( length * sizeof(uint32_t) );
	std::default_random_engine generator;
	std::uniform_int_distribution<uint32_t> distribution(0,max_capacity-1);
	uint32_t i,val;

	for(i=0;i<length;i++)
	{
		val = distribution(generator);
		requestsource[i] = val;
	}

	return requestsource;
}


uint32_t* RandomRequestSource::GenerateRandomPermutation(uint32_t length) {
	uint32_t* permutation = (uint32_t *) malloc( length * sizeof(uint32_t) );
	std::default_random_engine generator;
	std::uniform_int_distribution<uint32_t> distribution(0,length-1);
	uint32_t i,val;
  uint32_t temp;

  for(i=0; i<length; i++)
    permutation[i]=i;

	for(i=0;i<length;i++)
	{
		val = distribution(generator);
    temp = permutation[i];
    permutation[i]=permutation[val];
		permutation[val] = temp; 
	}
  printf("\n\n");

	return permutation;
}

double compute_stddev(double *elements, uint32_t num_elements){
  double mean = 0, var = 0, stddev;
  for(uint32_t i=0; i<num_elements; i++){
    mean+=elements[i];   
  }
  mean=(mean/num_elements);
  for(uint32_t i=0; i<num_elements; i++){
    double diff = mean - elements[i];
    var+=(diff*diff);
  }
  var=var/num_elements;
  stddev = sqrt(var);
  return stddev;
}

double compute_avg(double *elements, uint32_t num_elements){
  double mean = 0, var = 0, stddev;
  for(uint32_t i=0; i<num_elements; i++){
    mean+=elements[i];   
  }
  mean=(mean/num_elements);
  return mean;
}


double time_taken(timespec *start, timespec *end){
  long seconds, nseconds;
  seconds = end->tv_sec - start->tv_sec;
  nseconds = end->tv_nsec - start->tv_nsec;
  double mstime = ( double(seconds * 1000) + double(nseconds/MILLION) );
  return mstime;
}

// prepareDataBlock is meant to prepare Test Data blocks
// TestDataBlocks are of the format <Index><Padding with 0 upto data_size>

void prepareDataBlock(unsigned char *datablock, uint32_t index, uint32_t data_size){
  unsigned char *data_ptr = datablock;
  memcpy(data_ptr, (unsigned char*) &index, sizeof(uint32_t));
  for(uint32_t j = sizeof(uint32_t); j<data_size; j++){
    data_ptr[j]='0';
  }
  uint32_t index_stored;
  memcpy((unsigned char*) &index_stored, data_ptr, sizeof(uint32_t));
}

int checkFetchedDataBlock(unsigned char *datablock, uint32_t index, uint32_t data_size){
  unsigned char *data_ptr = datablock;
  uint32_t retrieved_index;
  memcpy((unsigned char*) &retrieved_index, data_ptr, sizeof(uint32_t));
  if(retrieved_index != index){
    printf("In CFDB: retrieved_index(%d) != index(%d)\n", retrieved_index, index);
    return 1;
  }
  for(uint32_t j=sizeof(uint32_t); j<data_size; j++){
    if(data_ptr[j]!='0'){
      printf("In CFDB: j=%d, data[j]!='0'\n",j);
      return 1;
    }
  }
  return 0;
}

void serializeRequest(uint32_t request_id, char op_type, unsigned char *data, uint32_t data_size, unsigned char* serialized_request){
  unsigned char *request_ptr = serialized_request;
  *request_ptr=op_type;
  request_ptr+=1;
  memcpy(request_ptr, (unsigned char*) &request_id,  ID_SIZE_IN_BYTES);
  request_ptr+=  ID_SIZE_IN_BYTES;	
  memcpy(request_ptr, data, data_size);
}

/*

Inputs: a target pub key, a seriailzed request and request size.
Outputs: instantiates and populates:
          client_pubkey, aes_key (from target_pubkey and generated client_pubkey ECDH)
          iv, encrypted request and tag for the request
*/


// [[ noreturn ]] void unreachable(std::string msg = "<No Message>") {
//     std::cerr << "Unreachable code reached. Message: " << msg << std::endl;
//     std::abort();
// }
