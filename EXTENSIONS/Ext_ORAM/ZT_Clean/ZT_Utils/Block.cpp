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

#include "Block.hpp"

Block::Block(){
	data = NULL;
	r = NULL;
	treeLabel = 0;
	id = 0;
}

Block::Block(uint32_t gN){
	data = NULL;
	r = NULL;
	treeLabel = 0;
	id = gN;
}

Block::Block(uint32_t data_size, uint32_t gN){
	data = NULL;
	r = NULL;
	generate_data(data_size);
	generate_r();
	treeLabel = 0;
	id = gN;
}

void Block::initialize(uint32_t data_size, uint32_t gN){
	data = NULL;
	r = NULL;
	generate_data(data_size);
	generate_r();
	treeLabel = 0;
	id = gN;
}

Block::Block(Block &b, uint32_t g_data_size){
	data = (unsigned char*) malloc (g_data_size);
	r = (uint8_t*) malloc(NONCE_LENGTH);
	memcpy(r,b.r,NONCE_LENGTH);
	memcpy(data,b.data,g_data_size);

	treeLabel = b.treeLabel;
	id = b.id;
}	

Block::Block(uint32_t set_id, uint8_t *set_data, uint32_t set_label){
	//data = ; Generate datablock of appropriate datasize for new block.
	//treeLabel = 0;
	id = set_id;
	data = set_data;
	treeLabel = set_label;
}

Block::Block(unsigned char* serialized_block, uint32_t blockdatasize){
	unsigned char* ptr = serialized_block;
	r = (uint8_t*) malloc(NONCE_LENGTH);

	memcpy(r,ptr,NONCE_LENGTH);
	ptr+= NONCE_LENGTH;

	memcpy((void *)&id, ptr, ID_SIZE_IN_BYTES);
	ptr+=ID_SIZE_IN_BYTES;
	memcpy((void *)&treeLabel, ptr, ID_SIZE_IN_BYTES);
	ptr+=ID_SIZE_IN_BYTES;

	data = (unsigned char*) malloc (blockdatasize);
	memcpy(data,ptr,blockdatasize);
	ptr+= blockdatasize;
}

Block::~Block()
{
	if(r)
		free(r);
	if(data)
		free(data);
}

void Block::generate_data(uint32_t fdata_size)
{
	// srand(time(NULL));
	// TODO : Fix to initiate with some value
	if(data==NULL)
		data = (uint8_t*) malloc (fdata_size);

	#ifdef RAND_DATA
		// sgx_read_rand(data,fdata_size);	
		// arc4random_buf(data, fdata_size);
		for (int i=0; i<fdata_size; i++){
			data[i] = rand();
		}
	#else
		for(uint32_t i=0; i<fdata_size; i++) {
			data[i] = (i % 26) + 65; // ASCII 'A'
		}
    data[fdata_size-1]='\0';
	#endif	
}

void Block::reset(uint32_t data_size, uint32_t gN)
{
  id = gN;
  treeLabel = 0;

  if(data==NULL)
    data=(uint8_t*) malloc(data_size);
  
		for(uint32_t i=0; i<data_size; i++) {
			data[i] = (i % 26) + 65; // ASCII 'A'
		}
    data[data_size-1]='\0';
}

void Block::generate_r()
{
	// srand(time(NULL));
	if(r==NULL)
		r = (uint8_t*) malloc(NONCE_LENGTH);

	#ifdef RAND_DATA
		// sgx_read_rand(r, NONCE_LENGTH);
		// arc4random_buf(r, NONCE_LENGTH);
		for (int i=0; i<NONCE_LENGTH; i++){
			r[i] = rand();
		}
	#else
		for(uint8_t i=0; i<NONCE_LENGTH; i++){
			r[i]=0;
		}
	#endif
}

bool Block::isDummy(uint32_t gN) {
	return (id == gN);
}

void Block::fill(unsigned char* serialized_block, uint32_t data_size){
	unsigned char* ptr = serialized_block;
	r = (uint8_t*) malloc(NONCE_LENGTH);

	memcpy(r,ptr,NONCE_LENGTH);
	ptr+= NONCE_LENGTH;

	memcpy((void *)&id, ptr, ID_SIZE_IN_BYTES);
	ptr+=ID_SIZE_IN_BYTES;
	memcpy((void *)&treeLabel, ptr, ID_SIZE_IN_BYTES);
	ptr+=ID_SIZE_IN_BYTES;

	data = (unsigned char*) malloc (data_size);
	memcpy(data,ptr,data_size);
	ptr+= data_size;
}

void Block::fill(uint32_t data_size){
	generate_data(data_size);
	generate_r();
}

void Block::fill_recursion_data(uint32_t *pmap, uint32_t recursion_data_size){
	memcpy(data,pmap,recursion_data_size);
}

unsigned char* Block::serialize(uint32_t data_size, bool setupstash) {
	uint32_t tdata_size = data_size + ADDITIONAL_METADATA_SIZE;
	unsigned char* serialized_block = (unsigned char*) malloc(tdata_size);
	unsigned char *ptr = serialized_block;
	unsigned char *nonceptr = serialized_block;
	uint64_t *nonceptr64 = (uint64_t*)nonceptr;
	unsigned char *enc_ptr;

	memcpy(ptr,(void *) r,NONCE_LENGTH);
	ptr+=NONCE_LENGTH;
	enc_ptr = ptr;

	memcpy(ptr,(void *) &id, sizeof(id));
	// #ifdef PROTECT_STASH_DEBUG
	// 	if(setupstash){
	// 		printf("initid: %d\n", *(uint32_t*)(ptr));
	// 	}
	// #endif
	ptr+=sizeof(id);
	memcpy(ptr,(void *) &treeLabel, sizeof(treeLabel));
	// #ifdef PROTECT_STASH_DEBUG
	// 	if(setupstash){
	// 		printf("initlabel: %d\n", *(uint32_t*)(ptr));
	// 	}
	// #endif
	ptr+=sizeof(treeLabel);
	memcpy(ptr,data,data_size);
	ptr+=data_size;
	
	
	if(setupstash){
		uint64_t *enc_ptr64 = (uint64_t*)enc_ptr;
		int i = 0;
  		uint32_t size = 0;
  		while(size<data_size+8){
    	enc_ptr64[i] = enc_ptr64[i]^nonceptr64[0];
    	size = size + 8;
    	i++;
    	if(size>=data_size+8){
     		break;
    	}
    	enc_ptr64[i] = enc_ptr64[i]^nonceptr64[1];
    	size = size + 8;
    	i++;
  		}
	}
	
	// #ifdef PROTECT_STASH_DEBUG
	// 	if(setupstash){
	// 		// printf("Afterd: %d, without enc: %d\n", getId(serialized_block), *(uint32_t*)(serialized_block+16));
	// 		printf("Afterd: %d, without enc: %d\n", getTreeLabel(serialized_block), *(uint32_t*)(serialized_block+20));
	// 	}
	// #endif
	return serialized_block;
}


void Block::serializeToBuffer(unsigned char* serialized_block, uint32_t data_size) {
	unsigned char *ptr = serialized_block;
	memcpy(ptr,(void *) r,NONCE_LENGTH);
	ptr+=NONCE_LENGTH;
	memcpy(ptr,(void *) &id,sizeof(id));
	ptr+=sizeof(id);
	memcpy(ptr,(void *) &treeLabel,sizeof(treeLabel));
	ptr+=sizeof(treeLabel);
	memcpy(ptr,data,data_size);
	ptr+=data_size;
}

void Block::serializeForAes(unsigned char* buffer, uint32_t bDataSize) {
	memcpy(buffer, (void *) &id, sizeof(id));
	memcpy(buffer+ID_SIZE_IN_BYTES, (void *) &treeLabel,sizeof(treeLabel));
	memcpy(buffer+ID_SIZE_IN_BYTES*2, data, bDataSize);
}

void Block::displayBlock(){
	printf("(ID = %d, Label = %d)\n", id, treeLabel);
	if(r){	
		printf("Nonce = %s\n", r);
	}
	else{
		printf("r = NULL\n");
	}

	if(data){
		printf("Data = %s\n", data);
	}
	else{
		printf("DATA_PTR = NULL\n");
	}
}

void Block::xor_enc(uint32_t data_size){
	generate_r();
	uint32_t input_size = data_size + 2*ID_SIZE_IN_BYTES;
	// unsigned char *ctr = (unsigned char*) malloc (NONCE_LENGTH);
	unsigned char *input_buffer = (unsigned char*) malloc(input_size);
	unsigned char *ciphertext = (unsigned char*) malloc (input_size);
	serializeForAes(input_buffer, data_size);

	// memcpy(ctr, r, NONCE_LENGTH);

	uint32_t data_to_encrypt = data_size + 8;
  	uint32_t ctr = 0;

	for(int i =0; i<data_to_encrypt; i++){
		if(ctr >= NONCE_LENGTH){
		ctr=0;
		}
		ciphertext[i] = input_buffer[i]^r[ctr];
		ctr++;
	}

	memcpy((void *) &id, ciphertext, ID_SIZE_IN_BYTES);
	memcpy((void *) &treeLabel, ciphertext + ID_SIZE_IN_BYTES, ID_SIZE_IN_BYTES);
	memcpy(data, ciphertext + ID_SIZE_IN_BYTES*2, data_size);

	free(input_buffer);
	free(ciphertext);

}

void Block::aes_enc(uint32_t data_size, unsigned char *aes_key) {
	generate_r();
	uint32_t input_size = data_size + 2*ID_SIZE_IN_BYTES;		
	unsigned char *ctr = (unsigned char*) malloc (NONCE_LENGTH);
	unsigned char *ciphertext = (unsigned char*) malloc (input_size);
	unsigned char *input_buffer = (unsigned char*) malloc(input_size);
	serializeForAes(input_buffer,data_size);

	memcpy(ctr, r, NONCE_LENGTH);			
	uint32_t ret = 0;
	int out_len = 0;

	EVP_CIPHER_CTX *ctx;

	if(!(ctx = EVP_CIPHER_CTX_new())){
		unreachable("AES CTX init failed\n");
	}

	if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, aes_key, ctr)){
		unreachable("AES init failed\n");
	}

	ret = EVP_EncryptUpdate(ctx, ciphertext, &out_len, input_buffer, input_size);
	if(ret != 1){
		unreachable("Encrypt failed\n");
	}

	if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + out_len, &out_len)){
		unreachable("Enc_Final Failed\n");
	}

	memcpy((void *) &id, ciphertext, ID_SIZE_IN_BYTES);
	memcpy((void *) &treeLabel, ciphertext + ID_SIZE_IN_BYTES, ID_SIZE_IN_BYTES);
	memcpy(data, ciphertext + ID_SIZE_IN_BYTES*2, data_size);	

	EVP_CIPHER_CTX_free(ctx);
	free(input_buffer);
	free(ciphertext);
	free(ctr);

}

void Block::aes_dec(uint32_t data_size, unsigned char *aes_key){
	uint32_t ciphertext_size = data_size+ 2*ID_SIZE_IN_BYTES;		
	unsigned char *ctr = (unsigned char*) malloc (NONCE_LENGTH);
	unsigned char *ciphertext = (unsigned char*) malloc(ciphertext_size);
	unsigned char *input_buffer = (unsigned char*) malloc (ciphertext_size);
	memcpy(ctr, r, NONCE_LENGTH);	

	serializeForAes(ciphertext, data_size);
	uint32_t ret = 0;
	int len;

	EVP_CIPHER_CTX *ctx;

	if(!(ctx = EVP_CIPHER_CTX_new())){
		unreachable("AES CTX init failed\n");
	}

	if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, aes_key, ctr)){
		unreachable("AES init failed\n");
	}

	ret = EVP_DecryptUpdate(ctx, input_buffer, &len, ciphertext, ciphertext_size);
	if(ret != 1){
		unreachable("Decryption failed\n");
	}

	if(1 != EVP_DecryptFinal_ex(ctx, input_buffer + len, &len)){
		unreachable("Final Dec failed\n");
	}

	EVP_CIPHER_CTX_free(ctx);
	free(ctr);

	memcpy((void *) &id, input_buffer,ID_SIZE_IN_BYTES);
	memcpy((void *) &treeLabel, input_buffer + ID_SIZE_IN_BYTES,ID_SIZE_IN_BYTES);
	memcpy(data, input_buffer + ID_SIZE_IN_BYTES*2, data_size);

	free(input_buffer);
	free(ciphertext);
	free(ctr);

}

unsigned char* Block::getDataPtr(){
  return data;
}
