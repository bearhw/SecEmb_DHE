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

#include "ORAMTree.hpp"


ORAMTree::ORAMTree(){
  aes_key = (unsigned char*) malloc (KEY_LENGTH);
}


ORAMTree::~ORAMTree(){

}

void ORAMTree::print_stash_count(uint32_t level, uint32_t nlevel){
  uint32_t stash_oc;
  if(recursion_levels>0){
    stash_oc = recursive_stash[level].stashOccupancy();
    printf("Level : %d, stash_occupancy :%d\n",level,stash_oc);
    recursive_stash[level].displayStashContents(nlevel, level!=(recursion_levels-1));
  }			
}

void ORAMTree::print_pmap0(){
  uint32_t p = 0;
  printf("Pmap0 = \n");
  while(p<real_max_blocks_level[0]){
    printf("(%d,%d)\n",p,posmap[p]);
    p++;
  }
  printf("\n");
}


/*
  ORAMTree::BuildTreeLevel
  
  The ORAMTree at recursion_level = level  has to store 
  max_blocks_level[level] blocks in it.
   
  util_divisor = Z is a parameter that determines how packed
  the leaf nodes are at initialization, and effectively determines
  the height of the tree instantiated.
  
  The tree has ceil(max_blocks_level[level]/util_divisor) leaf nodes.
*/

uint32_t* ORAMTree::BuildTreeLevel(uint8_t level, uint32_t* prev_pmap){
  uint32_t tdata_size;
  uint32_t block_size;
  
  uint32_t util_divisor = Z;
  uint32_t pD_temp = ceil((double)max_blocks_level[level]/(double)util_divisor);
  uint32_t pD = (uint32_t) ceil(log((double)pD_temp)/log((double)2));
  uint32_t pN = (int) pow((double)2, (double) pD);
  uint32_t ptreeSize = 2*pN-1;
  //+1 to depth pD, since ptreeSize = 2 *pN	
  D_level[level] = pD+1;
  N_level[level] = pN;
  printf("In ORAMTree:BuildTreeLevel N_level[%d]=%d\n", level, N_level[level]);
  printf("In ORAMTree:BuildTreeLevel D_level[%d]=%d\n", level, D_level[level]);
  printf("\n\nBuildTreeLevel,\nLevel : %d, Params - D = %d, N = %d, treeSize = %d, x = %d\n",level,pD,pN,ptreeSize,x);

  #ifdef BUILDTREE_DEBUG				
    printf("\n\nBuildTreeLevel,\nLevel : %d, Params - D = %d, N = %d, treeSize = %d, x = %d\n",level,pD,pN,ptreeSize,x);
  #endif

  if(level==(recursion_levels-1) || level==0) {
    tdata_size = data_size;	
    block_size = (data_size+ADDITIONAL_METADATA_SIZE);		
  }
  else {	
    tdata_size = recursion_data_size;
    block_size = recursion_data_size + ADDITIONAL_METADATA_SIZE;
  }						

  uint32_t posmap_size = max_blocks_level[level];
  // printf("Level: %d Posmap Size: %d \n", level, posmap_size);
  uint32_t *posmap_l = NULL;

  #ifdef PROTECT_POSMAP
    //2 extra spaces for 64 bit NONCE
    //If posmap size is odd, add 1 32 bit int for padding (for XOR with 64 bit nonce in oarray_search_NEW)
    uint64_t randnonce = rand();
    uint32_t padflag = posmap_size&1;
    posmap_l = (uint32_t *) malloc((posmap_size + 2 + padflag) * sizeof(uint32_t));
    posmap_l[posmap_size] = pN+1; //Declared as 2^tree levels + 1
    memcpy(&posmap_l[posmap_size+padflag], &randnonce, sizeof(uint64_t));
  #else
    posmap_l = (uint32_t *) malloc((posmap_size) * sizeof(uint32_t));
  #endif

  printf("Posmap allocated size for level %d : %d\n", level, posmap_size);
  
  if(posmap_l==NULL) {
    printf("Failed to allocate\n");
  }

  uint32_t blocks_per_bucket_in_ll = real_max_blocks_level[level]/pN;

  #ifdef BUILDTREE_DEBUG
    printf("Posmap Size = %f MB\n",float(max_blocks_level[level]*sizeof(uint32_t))/(float(1024*1024)));
    for(uint8_t jk = 0;jk <recursion_levels; jk++) {
      printf("real_max_blocks_level[%d] = %d \n",jk,real_max_blocks_level[jk]);
    }
    printf("\n");	
    printf("pN = %d, level = %d, real_max_blocks_level[level] = %d, blocks_per_bucket_in_ll = %d\n",pN, level, real_max_blocks_level[level], blocks_per_bucket_in_ll);
  #endif

  uint32_t c = real_max_blocks_level[level] - (blocks_per_bucket_in_ll * pN);
  uint32_t cnt = 0;

  Bucket temp(Z);
  temp.initialize(tdata_size, gN);

  //Build Last Level of Tree
  uint32_t label = 0;
  for(uint32_t i = pN; i <= ptreeSize; i++) {
    
    temp.reset_blocks(tdata_size, gN);

    uint32_t blocks_in_this_bucket = blocks_per_bucket_in_ll;
    if(cnt < c) {
      blocks_in_this_bucket+=1;
      cnt+=1;
    }

    #ifdef BUILDTREE_DEBUG
      printf("Bucket : %d\n", i);
    #endif

    for(uint8_t q=0;q<blocks_in_this_bucket;q++) {	
      temp.blocks[q].id = label;
      //treeLabel will be the bucket_id of that leaf = nlevel[level] + leaf
      temp.blocks[q].treeLabel = (pN) + (i - pN);

      // if(temp.blocks[q].treeLabel<=2047){
      //   printf("\nDebugging BuildTree: Id: %d, Leaf: %d, Level: %d\n", label, temp.blocks[q].treeLabel, level);
      //   for(int j = 0; j<recursion_data_size; j++){
      //     printf("%d ", temp.blocks[q].data[j]);
      //   }
      //   printf("\n");
      // }

      // if(label==4005){
      //   printf("ID: %d | treelabel: %d level: %d\n", label, temp.blocks[q].treeLabel, level);
      // }

      if(level<recursion_levels-1 && level>0) { 	
        temp.blocks[q].fill_recursion_data(&(prev_pmap[(label)*x]), recursion_data_size);
        // uint32_t* blockdataptr = (uint32_t*)temp.blocks[q].data;
      
        // for(int i=0; i<(uint32_t)(recursion_data_size/4); i++){
        //   if(*blockdataptr <= 2047 && *blockdataptr!=0){
        //     printf("BlockData: %d\n", *blockdataptr);
        //   }
        //   blockdataptr++;
        // }
      }


      posmap_l[temp.blocks[q].id] = temp.blocks[q].treeLabel;
      #ifdef PROTECT_POSMAP
        if(recursion_levels>1){
          uint32_t temp_id = temp.blocks[q].id;
          bool divflag = temp_id & 1;
          posmap_l[temp_id] = (temp.blocks[q].treeLabel)^(posmap_l[posmap_size + divflag]);
        }
      #else
        posmap_l[temp.blocks[q].id] = temp.blocks[q].treeLabel;
      #endif
      
      label++;
      // if( temp.blocks[q].id==0 ){
      //   printf("\nDebugging BuildTree: Id: %d, Leaf: %d, Level: %d\n", temp.blocks[q].id, temp.blocks[q].treeLabel, level);
      // }	
    }

    #ifdef BUILDTREE_DEBUG
      for(uint8_t q=0; q<Z; q++){
        if(level<recursion_levels-1 && level>0) { 	
            printf("(%d, %d): ",temp.blocks[q].id, temp.blocks[q].treeLabel);
            for(uint8_t p=0;p<x;p++) {
              printf("%d,",(prev_pmap[(label*x)+p]));
            }
            printf("\n");
        }
        else{
            printf("(%d, %d): ",temp.blocks[q].id, temp.blocks[q].treeLabel);
            unsigned char *ptr = temp.blocks[q].getDataPtr();
             for(uint64_t p=0; p<data_size; p++) {
              printf("%c", ptr[p]);
            }
            printf("\n");
        }
      }
      printf("\n");
    #endif		

    #ifdef ENCRYPTION_ON
      temp.aes_encryptBlocks(tdata_size, aes_key);
    #endif
    
    //TODO: No need to create and release memory for serialized bucket again and again
    unsigned char *serialized_bucket = temp.serialize(tdata_size);
    uint8_t ret;

    //Upload Bucket
    uploadBucket_OCALL(instance_id, oram_type, serialized_bucket, Z*block_size ,i, block_size, level);
     
    free(serialized_bucket);
  }

  #ifdef PROTECT_POSMAP
    uint32_t loopsize64 = posmap_size/2 + (posmap_size&1);
    printf("Posmap size: %d\n", posmap_size);
    printf("Loop size: %d\n", loopsize64);
    uint64_t *posmap64 = (uint64_t*) posmap_l;
    for(int i = 0; i<loopsize64; i++){
      // printf("Here4 %llx", posmap64[i]);
      posmap64[i] = posmap64[loopsize64]^posmap64[i];
      // printf("Here5 %llx", posmap64[i]);
    }
    // printf("%llx \n", posmap64[loopsize64]);
  #endif

  //Build Upper Levels of Tree
  for(uint32_t i = pN - 1; i>=1; i--){
    temp.reset_blocks(tdata_size, gN);

    #ifdef BUILDTREE_DEBUG
      for(uint8_t q=0; q<Z; q++){
        if(level<recursion_levels-1 && level>0) { 	
            printf("(%d, %d): ",temp.blocks[q].id, temp.blocks[q].treeLabel);
            for(uint8_t p=0;p<x;p++) {
              printf("%d,",(prev_pmap[(label*x)+p]));
            }
            printf("\n");
        }
        else{
            printf("(%d, %d): ",temp.blocks[q].id, temp.blocks[q].treeLabel);
            unsigned char *ptr = temp.blocks[q].getDataPtr();
             for(uint64_t p=0; p<data_size; p++) {
              printf("%c", ptr[p]);
            }
            printf("\n");
        }
      }
    #endif

    #ifdef ENCRYPTION_ON
      temp.aes_encryptBlocks(tdata_size, aes_key);
    #endif	

    unsigned char *serialized_bucket = temp.serialize(tdata_size);
    uint8_t ret;

    //Upload Bucket 
    uploadBucket_OCALL(instance_id, oram_type, serialized_bucket, Z*block_size ,i, block_size, level);

    free(serialized_bucket);	
  }

  return(posmap_l);
}

void ORAMTree::BuildTreeRecursive(uint8_t level, uint32_t *prev_pmap){	
  if(level == 0) {
    #ifdef BUILDTREE_DEBUG
      printf("BUILDTREE_DEBUG : Level 0 :\n");				
    #endif
			
    uint32_t *posmap_l;
    
    if(recursion_levels==1) {
      posmap_l = BuildTreeLevel(level, NULL);
    }
    else {
      uint32_t blocks_for_this_level = real_max_blocks_level[level];
      posmap_l = (uint32_t *) malloc( blocks_for_this_level * sizeof(uint32_t) );
      printf("Level: %d, Posmap size: %d\n", level, blocks_for_this_level);
      memcpy(posmap_l, prev_pmap, real_max_blocks_level[level] * sizeof(uint32_t));
      // for(int i=0; i<blocks_for_this_level; i++){
      //   posmap_l[i] = prev_pmap[i*4];
      // }
      // printf("Prevpmap of 287: %d\n", prev_pmap[287]);
      // printf("Prevpmap of 1077: %d\n", prev_pmap[1077]);
      // printf("Prevpmap of 287: %d\n", prev_pmap[287]);
      D_level[level] = 0;
      N_level[level] = max_blocks_level[level];
      printf("New N_level: %d\n", N_level[level]);
		
    }
    posmap = posmap_l;
  }
  else {
    // printf("BuildTreeRecursive else condition\n");
    // printf("Calling BuildTreeLevel with params: level: %d\n", level);
    uint32_t *posmap_level = BuildTreeLevel(level, prev_pmap);
    // printf("CB: Level: %d, Posmap size: %d\n", level, real_max_blocks_level[level]);
    BuildTreeRecursive(level-1, posmap_level);
    free(posmap_level);
  }
}

        /*	
        //Testing Module :
        unsigned char *bucket_array = (unsigned char*) malloc(Z*block_size);
        unsigned char *hash = (unsigned char*) malloc(HASH_LENGTH);
        uint8_t rt;
        downloadObject(&rt, bucket_array, Z*block_size, i, hash, HASH_LENGTH,level,D_level[level]);
        Bucket temp2(bucket_array,data_size);
        //Bucket temp3(serialized_bucket, data_size);
        //printf("(%d,%d) \t",temp2.blocks[0].id,temp2.blocks[0].treeLabel);
        temp2.aes_decryptBlocks(data_size);
        temp.aes_decryptBlocks(data_size);
        printf("%d :",i);					
        printf("(%d,%d) - ",temp.blocks[0].id,temp.blocks[0].treeLabel);
        //printf("(%d,%d) - ",temp3.blocks[0].id,temp3.blocks[0].treeLabel);
        //uint32_t *buck_ptr = (uint32_t*)(bucket_array + 16);					
        //printf(" - (%d,%d) - ",*buck_ptr,*(buck_ptr+1));				
        printf("(%d,%d) \n",temp2.blocks[0].id,temp2.blocks[0].treeLabel);
        free(bucket_array);
        */


void ORAMTree::Initialize() {
  printf("RECURSION_LEVELS = %d\n", recursion_levels);
  N_level = (uint64_t*) malloc ((recursion_levels) * sizeof(uint64_t));
  D_level = (uint32_t*) malloc ((recursion_levels) * sizeof(uint64_t));
  recursive_stash = (Stash *) malloc((recursion_levels) * sizeof(Stash));    

  //Fix stash_size for each level
  // 2.19498 log2(N) + 1.56669 * lambda - 10.98615
  
  for(uint32_t i=0; i<recursion_levels; i++){
    //printf("recursion_level i=%d, gN = %d\n",i, gN);
    
    if(i!=recursion_levels-1 && recursion_levels!=1){
      printf("Recursive setup call was invoked, data_ size = %d level: %d \n", recursion_data_size, i);
      if(oblivious_flag)
        recursive_stash[i].setup(stash_size,recursion_data_size, gN);
      else
        recursive_stash[i].setup_nonoblivious(recursion_data_size, gN);
    }
    else{
      if(oblivious_flag){
        printf("Correct stash setup call was invoked, data_ size = %d \n", data_size);
        recursive_stash[i].setup(stash_size, data_size, gN);
        }
      else
        recursive_stash[i].setup_nonoblivious(data_size, gN);
    }        
  }

  printf("In ORAMTree::Initialize(), Before BuildTreeRecursive, gN = %d\n", gN); 
  BuildTreeRecursive(recursion_levels-1, NULL);
  printf("In ORAMTree::Initialize(), After BuildTreeRecursive\n");			

  uint32_t d_largest;
  if(recursion_levels==0)
    d_largest = D_level[0];
  else
    d_largest = D_level[recursion_levels-1];

  //printf("Initialize , d_largest == %d!\n",d_largest);

  //Allocate encrypted_path and decrypted_path to be the largest path sizes the ORAM would ever need
  //So that we dont have to have costly malloc and free within access()
  //Since ZT is currently single threaded, these are shared across all ORAM instances
  //Will have to redesign these to be comoponents of the ORAM_Instance class in a multi-threaded setting.
  //PerformMemoryAllocations()

  uint64_t largest_path_size = Z*(data_size+ADDITIONAL_METADATA_SIZE)*(d_largest);
  //printf("Z=%d, data_size=%d, d_largest=%d, Largest_path_size = %ld\n", Z, data_size, d_largest, largest_path_size);
  encrypted_path = (unsigned char*) malloc (largest_path_size);
  decrypted_path = (unsigned char*) malloc (largest_path_size);
  fetched_path_array = (unsigned char*) malloc (largest_path_size);
  serialized_result_block = (unsigned char*) malloc (data_size+ADDITIONAL_METADATA_SIZE);
}

uint8_t uploadPath_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* path_array, uint32_t path_size, uint32_t leaf_label, uint8_t level, uint32_t D_level) {
  LocalStorage *ls;
  if(oram_type==0) {
    auto search = ls_PORAM.find(instance_id); 
    if(search != ls_PORAM.end()) {
      ls = search->second;
    }
    ls->uploadPath(leaf_label, path_array, level, D_level);
  }
  else if(oram_type==1) {
	  // unreachable("In CORAM");
    auto search = ls_CORAM.find(instance_id); 
    if(search != ls_CORAM.end()) {
      ls = search->second;
    }
    ls->uploadPath(leaf_label, path_array, level, D_level);
  }
  return 1;
}

void oarray_search(uint32_t *array, uint32_t loc, uint32_t *leaf, uint32_t newLabel,uint32_t N_level) {
  for(uint32_t i=0;i<N_level;i++) {
    #ifdef NEW_OMOVE2
      omove_NEW(i,&(array[i]),loc,leaf,newLabel);
    #else
      omove(i,&(array[i]),loc,leaf,newLabel);
    #endif
  }
  // printf("Here leaf: %x, loc: %x, newLabel: %x\n", *leaf, loc, newLabel);

}

uint8_t uploadBucket_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* serialized_bucket, uint32_t bucket_size, uint32_t label, uint32_t size_for_level, uint8_t recursion_level) {
  LocalStorage *ls;
  if(oram_type==0) {
    auto search = ls_PORAM.find(instance_id); 
    if(search != ls_PORAM.end()) {
      ls = search->second;
      ls->uploadBucket(label, serialized_bucket, size_for_level, recursion_level);
    }else{
      //printf("Did NOT find corresponding backend in ls_PORAM\n");
    }

  }
  else if(oram_type==1) {
  	// unreachable("In CORAM2");
    auto search = ls_CORAM.find(instance_id); 
    if(search != ls_CORAM.end()) {
      ls = search->second;
    }
    ls->uploadBucket(label, serialized_bucket, size_for_level, recursion_level); 
  }
  return 1;
}

uint8_t downloadPath_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* path_array, uint32_t path_size, uint32_t leaf_label, uint8_t level, uint32_t D_level) {	
  LocalStorage *ls;
  if(oram_type==0) {
    auto search = ls_PORAM.find(instance_id); 
    if(search != ls_PORAM.end()) {
      ls = search->second;
    }
    ls->downloadPath(leaf_label, path_array, level, D_level);
   }
  else if(oram_type==1) {
   	// unreachable("In CORAM3");
        auto search = ls_CORAM.find(instance_id); 
    if(search != ls_CORAM.end()) {
      ls = search->second;
    }
    ls->downloadPath(leaf_label, path_array, level, D_level);
   }

  return 1;
}

uint8_t downloadBucket_OCALL(uint32_t instance_id, uint8_t oram_type, unsigned char* serialized_bucket, uint32_t bucket_size, uint32_t label, unsigned char* hash, uint32_t hashsize, uint32_t size_for_level, uint8_t recursion_level) {
  LocalStorage *ls;
  if(oram_type==0) {
    auto search = ls_PORAM.find(instance_id); 
    if(search != ls_PORAM.end()) {
      ls = search->second;
    }
    ls->downloadBucket(label, serialized_bucket, size_for_level, hash, hashsize, recursion_level);
   }
  else if(oram_type==1) {
   	// unreachable("In CORAM4");
    auto search = ls_CORAM.find(instance_id); 
    if(search != ls_CORAM.end()) {
      ls = search->second;
    }
    ls->downloadBucket(label, serialized_bucket, size_for_level, hash, hashsize, recursion_level);
   }
  return 1;
}

void ORAMTree::refreshnonce(unsigned char* dec_path, uint32_t level){
  uint32_t d = D_level[level];
  unsigned char* diffnonce = (unsigned char*)malloc(NONCE_LENGTH);
  uint32_t *nonceptr32 = (uint32_t*)diffnonce;
  unsigned char* xorpath_ptr = dec_path;

  for(int i = 0; i < NONCE_LENGTH/4; i++){
    nonceptr32[i] = rand();
  }
  
  uint64_t* nonceptr64 = (uint64_t*)diffnonce;
  uint32_t data_to_encrypt = data_size + ADDITIONAL_METADATA_SIZE;

  for(int i = 0; i<Z*d; i++){
    uint32_t size = 0;
    int j = 0;

    xordata64(xorpath_ptr, diffnonce, NONCE_LENGTH);

    uint32_t *idptr = getIdPtr(xorpath_ptr);
    *idptr = *idptr^*nonceptr32;

    uint32_t *label = getTreeLabelPtr(xorpath_ptr);
    *label = *label^(*((uint32_t*)(diffnonce+4)));

    xordata64(getDataPtr(xorpath_ptr), diffnonce, data_size, 1);
  
    
  }

  free(diffnonce);


}

void ORAMTree::xornonce(unsigned char* before_block, uint32_t data_size, unsigned char *after_block, bool flag) {

  unsigned char *nonce =  (unsigned char*) malloc (NONCE_LENGTH);
  unsigned char *nonceptr = nonce;
  uint64_t* after_block_ptr = (uint64_t*)(after_block + NONCE_LENGTH);
  uint64_t* before_block_ptr = (uint64_t*)(before_block + NONCE_LENGTH);
  // unsigned char* after_block_ptr = (unsigned char*)(after_block + NONCE_LENGTH);
  // unsigned char* before_block_ptr = (unsigned char*)(before_block + NONCE_LENGTH);
  uint64_t* nonceptr64 = (uint64_t*)nonce;
  uint32_t* nonceptr32 = (uint32_t*)nonce;


  // //Data size + leaf label(4) + id(4)
  uint32_t data_to_encrypt = data_size + 8;

  if(flag){
    //encrypt
    #ifdef RAND_DATA
      for(int i = 0; i < NONCE_LENGTH/4; i++){
        nonceptr32[i] = rand();
      }
    #endif
    memcpy(after_block, nonce, NONCE_LENGTH);
  }
  else{
    //decrypt
    memcpy(nonce, before_block, NONCE_LENGTH);

  }

  int i = 0;
  uint32_t size = 0;
  while(size<data_to_encrypt){
    after_block_ptr[i] = before_block_ptr[i]^nonceptr64[0];
    size = size + 8;
    i++;
    if(size>=data_to_encrypt){
      break;
    }
    after_block_ptr[i] = before_block_ptr[i]^nonceptr64[1];
    size = size + 8;
    i++;
  }

  free(nonce);

  // memcpy(after_block, before_block, data_size+8);
}

void ORAMTree::aes_dec_serialized(unsigned char* encrypted_block, uint32_t data_size, unsigned char *decrypted_block, unsigned char* aes_key){
  unsigned char *ctr = (unsigned char*) malloc (NONCE_LENGTH);
  unsigned char *encrypted_block_ptr = encrypted_block + NONCE_LENGTH;
  unsigned char *decrypted_block_ptr = decrypted_block + NONCE_LENGTH;
  memcpy(ctr, encrypted_block, NONCE_LENGTH);

  // 8 from 4 bytes for id and 4 bytes for treelabel
  uint32_t ciphertext_size = data_size + 8;
  EVP_CIPHER_CTX *ctx;

  uint32_t ret = 0;
  int len;

    /* Create and initialise the context */
  if(!(ctx = EVP_CIPHER_CTX_new())){
    unreachable("AES CTX init failed\n");
  }

  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, aes_key, ctr)){
      unreachable("AES init failed\n");
  }

  ret = EVP_DecryptUpdate(ctx, decrypted_block_ptr, &len, encrypted_block_ptr, ciphertext_size);
  if(ret != 1){
    unreachable("Decryption failed\n");
  }

  if(1 != EVP_DecryptFinal_ex(ctx, decrypted_block_ptr + len, &len)){
    unreachable("Final Dec failed\n");
  }

  EVP_CIPHER_CTX_free(ctx);
  free(ctr);
}

void ORAMTree::aes_enc_serialized(unsigned char* decrypted_block, uint32_t data_size, unsigned char *encrypted_block, unsigned char* aes_key) {
  //Add generate_randomness() for nonce.
  unsigned char *ctr =  (unsigned char*) malloc (NONCE_LENGTH);
  #ifdef RAND_DATA
    for(int i = 0; i < NONCE_LENGTH; i++){
      ctr[i] = rand();
    }
  #endif

  memcpy(encrypted_block, ctr, NONCE_LENGTH);
    
  unsigned char *decrypted_block_ptr = decrypted_block + NONCE_LENGTH;
  unsigned char *encrypted_block_ptr = encrypted_block + NONCE_LENGTH;

  uint32_t ret = 0;
  int out_len;
    
    // 8 from 4 bytes for id and 4 bytes for treelabel
  uint32_t input_size = data_size + 8;
  
  EVP_CIPHER_CTX *ctx;
  if(!(ctx = EVP_CIPHER_CTX_new())){
    unreachable("AES CTX init failed\n");
  }

  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, aes_key, ctr)){
    unreachable("AES init failed\n");
  }

  ret = EVP_EncryptUpdate(ctx, encrypted_block_ptr, &out_len, decrypted_block_ptr, input_size);
  if(ret != 1){
    unreachable("Encrypt failed\n");
  }

  if(1 != EVP_EncryptFinal_ex(ctx, encrypted_block_ptr + out_len, &out_len)){
    unreachable("Enc_Final Failed\n");
  }

  EVP_CIPHER_CTX_free(ctx);
  free(ctr);
}

void ORAMTree::decryptPath(unsigned char* path_array, unsigned char *decrypted_path_array, uint32_t num_of_blocks_on_path, uint32_t data_size) {
  unsigned char *path_iter = path_array;
  unsigned char *decrypted_path_iter = decrypted_path_array;

  for(uint32_t i =0;i<num_of_blocks_on_path;i++) {
    #ifdef ENCRYPTION_ON
      #ifdef ENC_XOR
        xornonce(path_iter, data_size, decrypted_path_iter, 0);
      #else
        aes_dec_serialized(path_iter, data_size, decrypted_path_iter, aes_key);
      #endif
    #endif

    path_iter +=(data_size+ADDITIONAL_METADATA_SIZE);
    decrypted_path_iter +=(data_size+ADDITIONAL_METADATA_SIZE);
  }
}

void ORAMTree::encryptPath(unsigned char* path_array, unsigned char *encrypted_path_array, uint32_t num_of_blocks_on_path, uint32_t data_size) {
  unsigned char *path_iter = path_array;
  unsigned char *encrypted_path_iter = encrypted_path_array;

  for(uint32_t i =0;i<num_of_blocks_on_path;i++) {
    #ifdef ENCRYPTION_ON
      #ifdef ENC_XOR
        xornonce(path_iter, data_size, encrypted_path_iter, 1);
      #else
        aes_enc_serialized(path_iter, data_size, encrypted_path_iter, aes_key);
      #endif
    #endif
    path_iter +=(data_size + ADDITIONAL_METADATA_SIZE);
    encrypted_path_iter +=(data_size + ADDITIONAL_METADATA_SIZE);
  }
}


void ORAMTree::uploadPath(uint32_t leaf, unsigned char *path, uint64_t path_size, uint8_t level){
  uint32_t d = D_level[level];
  uint8_t ret;

  // #ifdef ENCRYPTION_ON
  //   uploadPath_OCALL(instance_id, oram_type, encrypted_path, path_size, leaf, level, d);
  // #else
  //   uploadPath_OCALL(instance_id, oram_type, decrypted_path, path_size, leaf, level, d);
  // #endif
  uploadPath_OCALL(instance_id, oram_type, path, path_size, leaf, level, d);
  
}

void ORAMTree::printPath(uint32_t leaf, uint8_t level)
{
  LocalStorage *ls;
  uint32_t d = D_level[level];
  // printf("DVal: %d\n", d);
  
  if(oram_type==0) {
    auto search = ls_PORAM.find(instance_id); 
    if(search != ls_PORAM.end()) {
      ls = search->second;
    }
    ls->printPath_ls(leaf, level, d);
  }
  else if(oram_type==1) {
    auto search = ls_CORAM.find(instance_id); 
    if(search != ls_CORAM.end()) {
      ls = search->second;
    }
    ls->printPath_ls(leaf, level, d);
  }
}

void ORAMTree::printPath_buffer(unsigned char* path, uint32_t length, uint32_t data_size){
  unsigned char* path_ptr = path;
  uint32_t *nonceptr = (uint32_t*)path_ptr;
  for(int i=0; i<length; i++){
    printf("\n");
    printf("Bucket: %d TreeLabel: %d\n", i, getTreeLabel(path_ptr));
    for(int j=0; j<4; j++){
      printf("Block: %d BlockID: %d TreeLabel: %d\n", j, getId(path_ptr), getTreeLabel(path_ptr));
      path_ptr = path_ptr + data_size + ADDITIONAL_METADATA_SIZE;
    }
  }

}

void ORAMTree::print_last_level(uint8_t level, uint32_t start, uint32_t end){
  LocalStorage *ls;

  if(oram_type==0) {
    auto search = ls_PORAM.find(instance_id); 
    if(search != ls_PORAM.end()) {
      ls = search->second;
    }
    ls->print_last_level_ls(level, start, end);
   }

}

void ORAMTree::xordata64(unsigned char* data, unsigned char* nonce, uint32_t data_size, bool blockdata_flag/*=0*/){

	uint64_t *data64 = (uint64_t*)data;
	uint64_t *nonce64 = (uint64_t*)nonce;
	int i = 0;
	uint32_t size = 0;

	while(size<data_size){
		data64[i] = data64[i]^nonce64[blockdata_flag];
		size = size + 8;
		i++;
		if(size>=data_size){
			break;
		}
		data64[i] = data64[i]^nonce64[!blockdata_flag];
		size = size + 8;
		i++;
	}

	
}


//For non-recursive level = 0
unsigned char* ORAMTree::downloadPath(uint32_t leaf, uint8_t level) {
  uint32_t temp = leaf;
  uint8_t rt;
  uint32_t tdata_size;
  uint32_t path_size;
  uint32_t d = D_level[level]; 

  // printf("Downloading Path: Level: %d Leaf: %d\n", leaf, level);

  if(level==recursion_levels-1||recursion_levels==1) {
      tdata_size = data_size;
  }
  else {
    tdata_size = recursion_data_size;
  }
  path_size = Z * (tdata_size+ADDITIONAL_METADATA_SIZE)* (d);

  #ifdef EXITLESS_MODE
    //while( !(*(req_struct->block)) ) {}
    *(req_struct->id) = leaf;
    *(req_struct->level) = level;
    *(req_struct->d_lev) = D_level[level];
    *(req_struct->recursion) = true;
    *(req_struct->block) = false;			

    while( !(*(req_struct->block)) ) {} // Wait til spinlock is set to true by RequestHandler thread

    fetched_path_array = resp_struct->path;
    // NOTE DO NOT FREE THESE IN EXITLESS MODE
    //Set path_array from resp_struct					
  #else
    downloadPath_OCALL(instance_id, oram_type, fetched_path_array, path_size, leaf, level, d);
  #endif

  #ifdef ACCESS_DEBUG
    printf("Verified path \n");
  #endif

  #ifndef PROTECT_STASH
    #ifdef ENCRYPTION_ON
      decryptPath(fetched_path_array,decrypted_path,(Z*(d)),tdata_size);
    #else
      decrypted_path = fetched_path_array;			
    #endif
  #else
    downloaded_path = fetched_path_array;
  #endif

  #ifdef ACCESS_DEBUG
    printf("Decrypted path \n");
  #endif

  #ifndef PROTECT_STASH
    #ifdef ENCRYPTION_ON
      return decrypted_path;
    #else
      return fetched_path_array;
    #endif
  #else
    return downloaded_path;
  #endif
}

void ORAMTree::PushBlocksFromPathIntoStash(unsigned char* decrypted_path_ptr, uint8_t level, uint32_t data_size, uint32_t block_size, uint32_t id, uint32_t position_in_id, uint32_t leaf, uint32_t *nextLeaf, uint32_t newleaf, uint32_t sampledLeaf, int32_t newleaf_nextlevel) {
  uint32_t d = D_level[level];
  uint32_t i;
  #ifdef ACCESS_DEBUG
    printf("Fetched Path in PushBlocksFromPathIntoStash, data_size = %d, leaf = %d : \n", data_size, leaf);
    //showPath(decrypted_path, Z, d, data_size);
    showPath_reverse(decrypted_path, Z, d, data_size, leaf);
  #endif

  // printf("Block size: %d Data size: %d\n", block_size, data_size);

  // FetchBlock Module :
  for(i=0;i< (Z*(d)); i++) {
    bool dummy_flag = getId(decrypted_path_ptr)==gN;

    if(oblivious_flag) {
      #ifdef ACCESS_DEBUG
        if(!dummy_flag){
          printf("PushBlocks ID: %d, Label: %d\n", getId(decrypted_path_ptr), getTreeLabel(decrypted_path_ptr));
        }
      #endif
      recursive_stash[level].pass_insert(decrypted_path_ptr, isBlockDummy(decrypted_path_ptr, gN));
      setId(decrypted_path_ptr, gN);
      // printf("ID Afer push: %d\n", getId(decrypted_path_ptr));
    }
    else {
      if(!(isBlockDummy(decrypted_path_ptr,gN))) {
        if(getId(decrypted_path_ptr) == id){
          setTreeLabel(decrypted_path_ptr, newleaf);

          //NOTE: if write operator, Write new data to block here.
          if(level!=recursion_levels) {
            uint32_t* temp_block_ptr = (uint32_t*) getDataPtr(decrypted_path_ptr);
            *nextLeaf = temp_block_ptr[position_in_id];
            if(*nextLeaf > gN || *nextLeaf < 0) {
              //Pull a random leaf as a temp fix.
              *nextLeaf = sampledLeaf;
              //printf("NEXTLEAF : %d, replaced with: %d\n",nextLeaf,newleaf_nextlevel);
            }
            temp_block_ptr[position_in_id] = newleaf_nextlevel;
          }
        }	
        recursive_stash[level].insert(decrypted_path_ptr);
      } 	
    }
    decrypted_path_ptr+=block_size;
  }	

  #ifdef ACCESS_DEBUG
    //printf("End of PushBlocksFromPathIntoStash, Path : \n");
    //showPath_reverse(decrypted_path, Z, d, data_size);
  #endif

}

//Scan over the stash and fix recustion leaf label
void ORAMTree::OAssignNewLabelToBlock(uint32_t id, uint32_t position_in_id, uint8_t level, uint32_t newleaf, uint32_t newleaf_nextlevel, uint32_t * nextLeaf){
  uint32_t k;
  nodev2 *listptr_t;
  listptr_t = recursive_stash[level].getStart();

  // printf("In OAssignNewLabeltoBlock\n");		
  
  for(k=0; k < stash_size; k++) {
    bool flag1,flag2 = false;
    flag1 = ( (getId(listptr_t->serialized_block) == id) && (!isBlockDummy(listptr_t->serialized_block,gN)) );

		oassign_newlabel(getTreeLabelPtr(listptr_t->serialized_block), newleaf, flag1);

    #ifdef ACCESS_DEBUG
      if(level != recursion_levels && recursion_levels!=-1){
      //printf("Block %d contents : ", getId(listptr_t->serialized_block));
      if(getId(listptr_t->serialized_block) == id)
        printf(" New Treelabel = %d\n", getTreeLabel(listptr_t->serialized_block));
        //for(uint8_t p = 0;p< recursion_block_size/4;p++) {
        //	printf("%d,",listptr_t->block->data[p*4]);
        //}
        //printf("\n");
      }
    #endif

    if(level!=recursion_levels && recursion_levels!=-1) {
      for(uint8_t p = 0;p < x;p++) {
        flag2 = (flag1 && (position_in_id == p));
        ofix_recursion( &(listptr_t->serialized_block[24+p*4]), flag2, newleaf_nextlevel, nextLeaf);
        /*
        #ifdef ACCESS_DEBUG						
          if(getId(listptr_t->serialized_block) == id) {
            for(uint8_t p = 0;p< recursion_block_size/4;p++) {
              printf("%d,",listptr_t->serialized_block[24+p*4]);
            }
            printf(", nextleaf = %d, flagr = %d\n", *nextLeaf, flagr);
          }
        #endif
        */
      }
    }
    listptr_t=listptr_t->next;
  }		
}

//PathORAM_Access(opType, id_adj, id,leaf, newleaf, newleaf_nextleaf,arr_blocks,  path_hash,level,D_level[level],N_level[level]);

/*
uint32_t ORAMTree::access_oram_level(char opType, uint32_t leaf, uint32_t id, uint32_t position_in_id, uint32_t level, uint32_t newleaf,uint32_t newleaf_nextleaf, unsigned char *data_in,  unsigned char *data_out)
{
    uint32_t return_value=-1;
    #ifdef EXITLESS_MODE			
        path_hash = resp_struct->path_hash;
    #endif

    decrypted_path = ReadBucketsFromPath(leaf + N_level[level], path_hash, level);
  
    switch(oram_controller){
        case PATHORAM : return_value = PathORAM_Access(opType, id, position_in_id,leaf, newleaf, newleaf_nextleaf,decrypted_path, 
                                path_hash,level,D_level[level],N_level[level], data_in, data_out); 
                break;
        case CIRCUITORAM: return_value = CircuitORAM_Access(opType, id, position_in_id, leaf, newleaf, newleaf_nextleaf,decrypted_path, 
                                    path_hash,level,D_level[level],N_level[level], data_in, data_out); 
                break;			
    }

    return return_value;		
}
*/
// if target[i] != _|_, then one block should be moved from path[i] to path[target[i]]


//Debug Function to display the count and     stash occupants

void ORAMTree::showPath(unsigned char *decrypted_path, uint8_t Z, uint32_t d, uint32_t data_size) {	
  unsigned char *decrypted_path_iter = decrypted_path;
  uint32_t block_size = data_size + ADDITIONAL_METADATA_SIZE;
  uint32_t num_of_blocks_on_path = Z*d;
  printf("\n\nshowPath() (From leaf to root) (Z=%d):  \n", Z);

  if(data_size == recursion_data_size) {
    for(uint32_t i = 0; i<num_of_blocks_on_path; i++) {
      if(i%Z==0){
        printf("\n");
      }
      printf("(%d,%d) :", getId(decrypted_path_iter), getTreeLabel(decrypted_path_iter));
      uint32_t no = (data_size)/sizeof(uint32_t);
      uint32_t* data_iter = (uint32_t*) (decrypted_path_iter + ADDITIONAL_METADATA_SIZE);
      unsigned char *data_ptr = (unsigned char*) (decrypted_path_iter + ADDITIONAL_METADATA_SIZE);

      if(getId(decrypted_path_iter)==gN){
          for(uint8_t q =0; q<data_size; q++)
            printf("%c", data_ptr[q]);
        }
        else{
          for(uint8_t q = 0;q<no;q++)
            printf("%d,",data_iter[q]);
        }
      printf("\n");
      decrypted_path_iter+=block_size;
    }
  } else {
    for(uint32_t i = 0;i<num_of_blocks_on_path;i++) {
      if(i%Z==0){
        printf("\n");
      }
      printf("(%d,%d) :",getId(decrypted_path_iter),getTreeLabel(decrypted_path_iter));
      unsigned char* data_ptr = decrypted_path_iter + ADDITIONAL_METADATA_SIZE;
      for(uint8_t q =0; q<data_size; q++)
        printf("%c", data_ptr[q]);
      printf("\n");
      decrypted_path_iter+=(block_size);
    }
  }
}

void ORAMTree::showPath_BlockIDLabel(unsigned char *decrypted_path, uint8_t Z, uint32_t d, uint32_t data_size, uint32_t bucket_id_of_leaf){
  printf("\n\nshowPath (Root to leaf): \n");
  uint32_t block_size = data_size + ADDITIONAL_METADATA_SIZE;
  printf("Data Size: %d, Block Size: %d, d: %d\n", data_size, block_size, d);
  unsigned char *decrypted_path_iter = decrypted_path + ((uint64_t)((Z*(d-1))) * uint64_t(block_size));
  uint32_t temp = bucket_id_of_leaf;
  uint32_t bucket_ids[d];
  printf("Bucket IDs: ");
  for(uint32_t i=0; i<d; i++){
    bucket_ids[i]=(temp>>(d-i-1));
    printf("%d ", bucket_ids[i]);
  }
  printf("\n");

  if(data_size == recursion_data_size ) {
    for(uint32_t i = 0; i < d; i++) {
      unsigned char *bucket_iter = decrypted_path_iter;
 
      printf("Bucket id %d:\n", bucket_ids[i]);
      for(uint32_t j = 0; j < Z; j++){
        printf("(%d,%d)\n",getId(bucket_iter), getTreeLabel(bucket_iter));
        bucket_iter+= block_size;
      }
      printf("\n");
      decrypted_path_iter-=(Z*block_size);
    }
  }
  else {
    for(uint32_t i = 0; i<d; i++) {
      unsigned char* bucket_iter = decrypted_path_iter;

      printf("Bucket id %d:\n", bucket_ids[i]);
      temp=temp>>1;
      for(uint32_t j =0; j<Z; j++){

        printf("(%d,%d), Add: %llu \n",getId(bucket_iter),getTreeLabel(bucket_iter), &(*bucket_iter));

        bucket_iter+=(block_size);
      }
      printf("\n");
      decrypted_path_iter-=(Z*block_size);
    }
  }

}


//Debug Function to show a tree path in reverse
void ORAMTree::showPath_reverse(unsigned char *decrypted_path, uint8_t Z, uint32_t d, uint32_t data_size, uint32_t bucket_id_of_leaf) {
  printf("\n\nshowPath_reverse (Root to leaf): \n");
  uint32_t block_size = data_size + ADDITIONAL_METADATA_SIZE;
  unsigned char *decrypted_path_iter = decrypted_path + ((uint64_t)((Z*(d-1))) * uint64_t(block_size));
  uint32_t temp = bucket_id_of_leaf;
  uint32_t bucket_ids[d];
  for(uint32_t i=0; i<d; i++){
    bucket_ids[i]=(temp>>(d-i-1));
  }

  if(data_size == recursion_data_size ) {
    for(uint32_t i = 0; i < d; i++) {
      unsigned char *bucket_iter = decrypted_path_iter;
 
      printf("Bucket id %d:\n", bucket_ids[i]);
      for(uint32_t j = 0; j < Z; j++){
        printf("(%d,%d) :",getId(bucket_iter), getTreeLabel(bucket_iter));
        uint32_t no = (data_size)/sizeof(uint32_t);
        uint32_t* data_iter = (uint32_t*) (bucket_iter + ADDITIONAL_METADATA_SIZE);
        unsigned char *data_ptr = (unsigned char*) (bucket_iter + ADDITIONAL_METADATA_SIZE);

        if(getId(bucket_iter)==gN){
          for(uint8_t q =0; q<data_size; q++)
            printf("%c", data_ptr[q]);
        }
        else{
          for(uint8_t q = 0;q<no;q++)
            printf("%d,",data_iter[q]);
        }

        printf("\n");
        bucket_iter+= block_size;
      }
      printf("\n");
      decrypted_path_iter-=(Z*block_size);
    }
  }
  else {
    for(uint32_t i = 0; i<d; i++) {
      unsigned char* bucket_iter = decrypted_path_iter;

      printf("Bucket id %d:\n", bucket_ids[i]);
      temp=temp>>1;
      for(uint32_t j =0; j<Z; j++){
        printf("(%d,%d) :",getId(bucket_iter),getTreeLabel(bucket_iter));

        unsigned char* data_ptr = (unsigned char*) (bucket_iter + ADDITIONAL_METADATA_SIZE);
        for(uint8_t q =0; q<data_size; q++)
          printf("%c", data_ptr[q]);

        printf("\n");
        bucket_iter+=(block_size);
      }
      printf("\n");
      decrypted_path_iter-=(Z*block_size);
    }
  }
}

void ORAMTree::SetParams(uint32_t s_instance_id, uint8_t s_oram_type, uint8_t pZ, uint32_t s_max_blocks, uint32_t s_data_size, uint32_t s_stash_size, uint32_t oblivious, uint32_t s_recursion_data_size, uint8_t precursion_levels){
  printf("IN ORAMTree::SetParams!!\n");
  instance_id = s_instance_id;
  oram_type = s_oram_type;
  data_size = s_data_size;
  stash_size = s_stash_size;
  oblivious_flag = (oblivious==1);
  recursion_data_size = s_recursion_data_size; 
  recursion_levels = precursion_levels;
  x = recursion_data_size/sizeof(uint32_t);
  Z = pZ;
       
  uint64_t size_pmap0 = s_max_blocks * sizeof(uint32_t);
  uint64_t cur_pmap0_blocks = s_max_blocks;
  while(size_pmap0 > MEM_POSMAP_LIMIT) {
    cur_pmap0_blocks = (uint64_t) ceil((double)cur_pmap0_blocks/(double)x);
    size_pmap0 = cur_pmap0_blocks * sizeof(uint32_t);
  }
 
  max_blocks_level = (uint64_t*) malloc((recursion_levels) * sizeof(uint64_t));
  real_max_blocks_level = (uint64_t*) malloc((recursion_levels) * sizeof(uint64_t));

  uint8_t max_recursion_level_index = recursion_levels-1;
  real_max_blocks_level[max_recursion_level_index] = s_max_blocks;
  int32_t lev = max_recursion_level_index-1;
  while(lev >= 0) {
    real_max_blocks_level[lev] = ceil((double)real_max_blocks_level[lev+1]/(double) x);
    lev--;
  }	

  #ifdef SET_PARAMETERS_DEBUG
    for(uint8_t j = 0; j <= max_recursion_level_index; j++) {
      printf("real_max_blocks_level[%d] = %d \n",j,real_max_blocks_level[j]);
    }
    printf("\n");	
  #endif
            
  max_blocks_level[0] = cur_pmap0_blocks;
  for(uint32_t i = 1; i <= max_recursion_level_index; i++) {			
    max_blocks_level[i] = max_blocks_level[i-1] * x;
  }

  if(recursion_levels >= 2){
    real_max_blocks_level[0] = real_max_blocks_level[1];
  }

  
  
  #ifdef SET_PARAMETERS_DEBUG
    for(uint32_t i = 0; i <= max_recursion_level_index; i++) {			
      printf("ENCLAVE:Level : %d, Blocks : %d\n", i, max_blocks_level[i]);
    }
    printf("\n");
  #endif

  gN = max_blocks_level[max_recursion_level_index];
  
}

