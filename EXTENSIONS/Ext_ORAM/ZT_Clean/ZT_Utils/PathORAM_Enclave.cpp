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

#include "PathORAM_Enclave.hpp"

#ifdef DETAILED_MICROBENCHMARKER
  det_mb_params DET_MB_PARAMS;
  det_mb ***MB = NULL;
  uint32_t req_counter=0;
#endif

/*
PathORAM::PathORAM(uint32_t s_max_blocks, uint32_t s_data_size, uint32_t s_stash_size, uint32_t oblivious, uint32_t s_recursion_data_size, int8_t recursion_levels, uint64_t onchip_posmap_mem_limit){
  max_blocks = s_max_blocks;
  data_size = s_data_size;
  stash_size = s_stash_size;
  oblivious_flag = (oblivious==1);
  recursion_data_size = s_recursion_data_size;
  mem_posmap_limit = onchip_posmap_mem_limit;  
};
*/

uint32_t* PathORAM::GenerateRandomSequence(uint32_t length, uint32_t start, uint32_t num_blocks) {
	uint32_t* newleafarr = (uint32_t *) malloc( length * sizeof(uint32_t) );
	std::default_random_engine generator;
	std::uniform_int_distribution<uint32_t> distribution(start,num_blocks-1);
	uint32_t i,val;

	for(i=0;i<length;i++)
	{
		val = distribution(generator);
		newleafarr[i] = val;
	}

	return newleafarr;
}


void PathORAM::Initialize(uint32_t instance_id, uint8_t oram_type, uint8_t pZ, uint32_t pmax_blocks, uint32_t pdata_size, uint32_t pstash_size, uint32_t poblivious_flag, uint32_t precursion_data_size, uint8_t precursion_levels){
  srand(time(NULL));
  ORAMTree::SetParams(instance_id, oram_type, pZ, pmax_blocks, pdata_size, pstash_size, poblivious_flag, precursion_data_size, precursion_levels);
  ORAMTree::Initialize();
  // newleaf_arr = GenerateRandomSequence(50000, 16384, 32767);
  // newleaf_ctr = 0;

}

double timetaken(timespec *start, timespec *end) {
  long seconds, nseconds;
  seconds = end->tv_sec - start->tv_sec;
  nseconds = end->tv_nsec - start->tv_nsec;
  double mstime = ( double(seconds * 1000) + double(nseconds/MILLION) );
  return mstime;
}

void time_report(int report_type, uint8_t level) {
  //Compute based on report_type and update MB.

  clockid_t clk_id = CLOCK_PROCESS_CPUTIME_ID;
  static struct timespec start, end;
  
  #ifdef DETAILED_MICROBENCHMARKER
    if(DET_MB_PARAMS.on == true) {

      if(DET_MB_PARAMS.oram_type==0) {
        //PathORAM part
        if(report_type==PO_POSMAP_START) {
          clock_gettime(clk_id, &start); 
        }
   
        if(report_type==PO_POSMAP_END) {
          clock_gettime(clk_id, &end); 
          double posmap_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][0];
          ptr->posmap_time = posmap_time;
        } 

        if(report_type==PO_DOWNLOAD_PATH_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==PO_DOWNLOAD_PATH_END) {
          clock_gettime(clk_id, &end); 
          double dp_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][level];
          ptr->download_path_time = dp_time;
        }

        if(report_type==PO_FETCH_BLOCK_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==PO_FETCH_BLOCK_END) {
          clock_gettime(clk_id, &end); 
          double fb_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][level];
          ptr->fetch_block_time = fb_time;
        }

        if(report_type==PO_EVICTION_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==PO_EVICTION_END) {
          clock_gettime(clk_id, &end); 
          double el_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][level];
          ptr->eviction_time = el_time;
        }

        if(report_type==PO_UPLOAD_PATH_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==PO_UPLOAD_PATH_END) {
          clock_gettime(clk_id, &end); 
          double up_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][level];
          ptr->upload_path_time = up_time;
        }
   
      }
      else if(DET_MB_PARAMS.oram_type==1) {
        //CircuitORAM part

        if(report_type==CO_POSMAP_START) {
          clock_gettime(clk_id, &start); 
        }
   
        if(report_type==CO_POSMAP_END) {
          clock_gettime(clk_id, &end); 
          double posmap_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][0];
          ptr->posmap_time = posmap_time;
        } 

        if(report_type==CO_DOWNLOAD_PATH_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==CO_DOWNLOAD_PATH_END) {
          clock_gettime(clk_id, &end); 
          double dp_time = timetaken(&start, &end);
          //printf("Download Time = %f, %d", dp_time, level);
          det_mb *ptr = MB[req_counter][level];
          ptr->download_path_time = dp_time;
        }

        if(report_type==CO_FETCH_BLOCK_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==CO_FETCH_BLOCK_END) {
          clock_gettime(clk_id, &end); 
          double fb_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][level];
          ptr->fetch_block_time = fb_time;
        }

        if(report_type==CO_UPLOAD_PATH_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==CO_UPLOAD_PATH_END) {
          clock_gettime(clk_id, &end); 
          double up_time = timetaken(&start, &end);
          //printf("Upload Time = %f\n", up_time);
          det_mb *ptr = MB[req_counter][level];
          ptr->upload_path_time = up_time;
        }

        if(report_type==CO_EVICTION_START) {
          clock_gettime(clk_id, &start); 
        } 

        if(report_type==CO_EVICTION_END) {
          clock_gettime(clk_id, &end); 
          double el_time = timetaken(&start, &end);
          det_mb *ptr = MB[req_counter][level];
          ptr->eviction_time = el_time;
        }

      }
    }
  #endif
}

void PathORAM::Create(uint32_t instance_id, uint8_t oram_type, uint8_t Z, uint32_t max_blocks, uint32_t data_size, uint32_t stash_size, uint32_t oblivious_flag, uint32_t recursion_data_size, uint8_t recursion_levels){
  Initialize(instance_id, oram_type, Z, max_blocks, data_size, stash_size, oblivious_flag, recursion_data_size, recursion_levels);
}

uint32_t PathORAM::access_oram_level(char opType, uint32_t leaf, uint32_t id, uint32_t position_in_id, uint32_t level, uint32_t newleaf,uint32_t newleaf_nextleaf, unsigned char *data_in,  unsigned char *data_out)
{
  uint32_t return_value=-1;

  #ifdef DETAILED_MICROBENCHMARKER
    time_report(PO_DOWNLOAD_PATH_START, level);
  #endif

  // printf("a_o_l : Level: %d Leaf: %d\n", level, leaf);
  decrypted_path = downloadPath(leaf, level);
  // printPath(leaf, level);

  #ifdef DETAILED_MICROBENCHMARKER
    time_report(PO_DOWNLOAD_PATH_END, level);
  #endif

  return_value = PathORAM_Access(opType, id, position_in_id, leaf, newleaf, newleaf_nextleaf,decrypted_path, level, data_in, data_out); 
  return return_value;		
}


uint32_t PathORAM::access(uint32_t id, int32_t position_in_id, char opType, uint8_t level, unsigned char* data_in, unsigned char* data_out, uint32_t* prev_sampled_leaf){
  uint32_t leaf = 0;
  uint32_t nextLeaf;
  uint32_t id_adj;				
  uint32_t newleaf;
  uint32_t newleaf_nextlevel = -1;
  unsigned char random_value[ID_SIZE_IN_BYTES];

  // if(newleaf_ctr > 49999){
  //   newleaf_ctr = 0;
  // }
  

  // printf("Rec level: %d, Level:%d\n", recursion_levels, level);


  if(recursion_levels ==  1) {
    level = 0;
    // sgx_status_t rt = SGX_SUCCESS;
    // rt = sgx_read_rand((unsigned char*) random_value,ID_SIZE_IN_BYTES);
    // uint32_t newleaf = (N_level[0]) + (*((uint32_t *)random_value) % N_level[0]);
    uint32_t newleaf = (N_level[0]) + ((rand()) % N_level[0]);

    // uint32_t newleaf = newleaf_arr[newleaf_ctr];
    // newleaf_ctr++;
    // printf("Newleaf: %d, Newleaf_ctr: %d\n", newleaf, newleaf_ctr);


    //rec_level = 0, because this is the no recursion_case.

    #ifdef DETAILED_MICROBENCHMARKER
      time_report(PO_POSMAP_START, 0);
    #endif

    if(oblivious_flag) {
      #ifdef PROTECT_POSMAP
        oarray_search_NEW(posmap, id, &leaf, newleaf, max_blocks_level[0]);		
      #else
        oarray_search(posmap, id, &leaf, newleaf, max_blocks_level[0]);		
      #endif
    }
    else{
      leaf = posmap[id];
      posmap[id] = newleaf;			
    }	

    #ifdef DETAILED_MICROBENCHMARKER
      time_report(PO_POSMAP_END, 0);
    #endif

    //Bucket_id = N_level[level] + leaf label


    #ifdef DETAILED_MICROBENCHMARKER
      time_report(PO_DOWNLOAD_PATH_START, 0);
    #endif

    decrypted_path = downloadPath(leaf, 0);		
    // printf("Downloaded Path: \n");
    // uint32_t d = D_level[level];
    // showPath_BlockIDLabel(decrypted_path, Z, d, data_size, leaf);	
 
    #ifdef DETAILED_MICROBENCHMARKER
      time_report(PO_DOWNLOAD_PATH_END, 0);
    #endif

    PathORAM_Access(opType, id, -1, leaf, newleaf, -1, decrypted_path, 0, data_in, data_out);
    return 0;
  }
  else{
    if(level==0) {
      // sgx_read_rand((unsigned char*) random_value, ID_SIZE_IN_BYTES);
      //To slot into one of the buckets of next level
      // newleaf = N_level[1] + (*((uint32_t *)random_value) % (N_level[level+1]));
      newleaf = (N_level[1]) + ((rand()) % N_level[level+1]);

      // uint32_t newleaf = newleaf_arr[newleaf_ctr];
      // newleaf_ctr++;
      // printf("Newleaf: %d, Newleaf_ctr: %d\n", newleaf, newleaf_ctr);
      // printf("Newleaf level0 = %d\n", newleaf);

      uint32_t id_temp = id;
      id_temp = id*x + position_in_id;



      #ifdef DETAILED_MICROBENCHMARKER
        time_report(PO_POSMAP_START, level);
      #endif

      // uint32_t id_temp = id*x + position_in_id;
      // printf("Before searching level: %d with blocks: %d\n", level, real_max_blocks_level[level]);
      if(oblivious_flag) {
        #ifdef PROTECT_POSMAP
          oarray_search_NEW(posmap, id_temp, &leaf, newleaf, real_max_blocks_level[level]);			
        #else
          oarray_search(posmap, id_temp, &leaf, newleaf, real_max_blocks_level[level]);			
        #endif
      }			
      else {
        leaf = posmap[id_temp];
        posmap[id_temp] = newleaf;
      }

      // printf("PAccess_If: ID: %d Level: %d Fetched leaf: %d\n", id, level, leaf);
    
      #ifdef DETAILED_MICROBENCHMARKER
        time_report(PO_POSMAP_END, level);
      #endif

      *prev_sampled_leaf = newleaf;

      #ifdef ACCESS_DEBUG
        printf("access : Level = %d: \n Requested_id = %d, Corresponding leaf from posmap = %d, Newleaf assigned = %d,\n\n",level,id,leaf,newleaf);
      #endif				
      return leaf;
    }
    else if(level == recursion_levels-1){
      //DataAccess for leaf.
      id_adj = id/x;
      position_in_id = id%x;
      // printf("PAccess Level: %d ID: %d Calling access with id: %d\n", level, id, id_adj);
      leaf = access(id_adj, position_in_id, opType, level-1, data_in, data_out, prev_sampled_leaf);	
      // printf("PAccess Level: %d Fetched leaf: %d\n", level, leaf);
      
      #ifdef ACCESS_DEBUG					
        printf("access, Level = %d:  before access_oram_level : Block_id = %d, Newleaf = %d, Leaf from level = %d, Flag = %d\n",level,id,*prev_sampled_leaf,leaf,oblivious_flag);
      #endif
      //ORAM ACCESS of recursion_levels is to fetch entire Data block, no position_in_id, hence -1)
      access_oram_level(opType, leaf, id, -1, level, *prev_sampled_leaf, -1, data_in, data_out);
      return 0;	
    }
    else {
      id_adj = id/x;
      int32_t pos_in_id = id%x;
      // printf("PAccess Level: %d ID: %d Calling access with id: %d\n", level, id, id_adj);
      leaf = access(id_adj, pos_in_id, opType, level-1, data_in, data_out, prev_sampled_leaf);
      // printf("PAccess Level: %d Fetched leaf: %d\n", level, leaf);
      
      //sampling leafs for a level ahead		
      // sgx_read_rand((unsigned char*) random_value, ID_SIZE_IN_BYTES);
      //Add rand
      // newleaf_nextlevel = N_level[level+1] + (*((uint32_t *)random_value) % N_level[level+1]);	
      newleaf_nextlevel = (N_level[level+1]) + ((rand()) % N_level[level+1]);				

      #ifdef ACCESS_DEBUG
        printf("access : Level = %d: \n leaf = %d, block_id = %d, position_in_id = %d, newleaf_nextlevel = %d\n",level,leaf,id,position_in_id,newleaf_nextlevel);
      #endif

      nextLeaf = access_oram_level(opType, leaf, id, position_in_id, level, *prev_sampled_leaf, newleaf_nextlevel, data_in, data_out);
      *prev_sampled_leaf = newleaf_nextlevel;

      #ifdef ACCESS_DEBUG
        printf("access, Level = %d (After ORAM access): nextLeaf from level = %d \n\n",level,nextLeaf);
      #endif
      return nextLeaf;
    }
  }
}	


void PathORAM::Access(uint32_t id, char opType, unsigned char* data_in, unsigned char* data_out){
  uint32_t prev_sampled_leaf=-1;
  access(id, -1, opType, recursion_levels-1, data_in, data_out, &prev_sampled_leaf);
}


uint32_t PathORAM::PathORAM_Access(char opType, uint32_t id, uint32_t position_in_id, uint32_t leaf, uint32_t newleaf, uint32_t newleaf_nextlevel, unsigned char* decrypted_path, uint32_t level, unsigned char* data_in, unsigned char *data_out) {
  uint32_t i, nextLeaf = 0;
  uint32_t d = D_level[level];
  uint32_t n = N_level[level];
  uint32_t sampledLeaf;
  bool flag = false;
  bool ad_flag = false;
  unsigned char *decrypted_path_ptr = decrypted_path;
  uint8_t rt;
  uint32_t randval;
  // unsigned char random_value[ID_SIZE_IN_BYTES];
  #ifdef RAND_DATA
    randval = rand();
  #endif
  // sgx_read_rand((unsigned char*) random_value, sizeof(uint32_t));
  // printf("In PathORAM Access with id: %d level: %d leaf: %d\n", id, level, leaf);

  #ifdef SHOW_STASH_COUNT_DEBUG
      uint32_t stash_oc;
      stash_oc = recursive_stash[level].stashOccupancy();
      printf("Level : %d , Just in Access:%d\n",level,stash_oc);		
  #endif

  if(recursion_levels!=1 && level!=recursion_levels-1){
    // sampledLeaf= N_level[level+1] + (*((uint32_t *)random_value) % (N_level[level+1]));
    sampledLeaf = (N_level[level+1]) + ((rand()) % (N_level[level+1]));
  }			
  else{
    sampledLeaf= n + (randval % (n));
    // sampledLeaf= n + (*((uint32_t *)random_value) % (n));
  }

  uint32_t tblock_size, tdata_size;
  if(recursion_levels==1||level==recursion_levels-1) {
      tblock_size = data_size + ADDITIONAL_METADATA_SIZE;
      tdata_size = data_size;	
  } 
  else {
    tblock_size = recursion_data_size + ADDITIONAL_METADATA_SIZE;				
    tdata_size = recursion_data_size;			
  }

  // showPath_BlockIDLabel(decrypted_path, Z, d, tdata_size, leaf);

    
  uint32_t path_size = Z*tblock_size*(d);


  #ifdef DETAILED_MICROBENCHMARKER
    time_report(PO_FETCH_BLOCK_START, level);
  #endif

  // printPath(16381, level);

  uint32_t stash_oc;
  // stash_oc = recursive_stash[level].stashOccupancy();
  // printf("Level: %d Before Push Blocks: %d\n", level, stash_oc);

  PushBlocksFromPathIntoStash(decrypted_path_ptr, level, tdata_size, tblock_size, id, position_in_id, leaf, &nextLeaf, newleaf, sampledLeaf, newleaf_nextlevel);          

  // printf("After Push Blocks\n");
  stash_oc = recursive_stash[level].stashOccupancy();
  // printf("Level : %d, After push stash_oc:%d\n",level,stash_oc);
  
  
  #ifdef SHOW_STASH_COUNT_DEBUG
    stash_oc = recursive_stash[level].stashOccupancy();
    printf("Level : %d, After push stash_oc:%d\n",level,stash_oc);
  #endif

  if(oblivious_flag) {            
    // if((level == recursion_levels-1) || level==1){
    if((level == recursion_levels-1)){
      // printf("Leaf: %d, newleaf: %d\n", leaf, newleaf);
      // printf("Calling PerformAccessoperation with ID: %d Newleaf: %d\n", id, newleaf);
      recursive_stash[level].PerformAccessOperation(opType, id, newleaf, data_in, data_out);
      // #endif
      //Optional TODO : Add layer of encryption to result, such that only real client (outside server stack) can decrypt.                
    }
    else{
      // printf("In OAssign\n");
      #ifdef PROTECT_STASH
        // recursive_stash[level].PerformAccessOperation2(opType, id, newleaf, data_in, data_out);
        recursive_stash[level].PerformAccessOperation2(opType, id, position_in_id,  newleaf, newleaf_nextlevel, &nextLeaf, recursion_data_size);
      #else
        OAssignNewLabelToBlock(id, position_in_id, level, newleaf, newleaf_nextlevel, &nextLeaf);
      #endif
      
    }
  }

  #ifdef DETAILED_MICROBENCHMARKER
    time_report(PO_FETCH_BLOCK_END, level);
  #endif

  #ifdef SHOW_STASH_CONTENTS
    recursive_stash[level].displayStashContents(n, level!=(recursion_levels-1));  
  #endif

  #ifdef SHOW_STASH_COUNT_DEBUG
    stash_oc = recursive_stash[level].stashOccupancy();
    printf("Level : %d, Before rebuild stash_oc:%d\n",level,stash_oc);
  #endif

  decrypted_path_ptr = decrypted_path;


  #ifdef DETAILED_MICROBENCHMARKER
    time_report(PO_EVICTION_START, level); 
  #endif

  // printf("Next Leaf before Rebuild Path: %d\n", nextLeaf);

  PathORAM_RebuildPath(decrypted_path_ptr, tdata_size, tblock_size, leaf, level);

  // printf("Final Path after PathORAM_RebuildPath: \n");
  // showPath_BlockIDLabel(decrypted_path, Z, d, tdata_size, leaf);
 
  #ifdef ACCESS_DEBUG
    printf("Final Path after PathORAM_RebuildPath: \n");
    showPath_reverse(decrypted_path, Z, d, tdata_size, leaf);
  #endif

  #ifdef SHOW_STASH_COUNT_DEBUG
      stash_oc = recursive_stash[level].stashOccupancy();
      printf("Level : %d , After rebuild stash_oc:%d\n",level,stash_oc);		
  #endif

  #ifdef SHOW_STASH_CONTENTS
    if(level==recursion_levels-1)
      recursive_stash[level].displayStashContents(n, false);
  #endif

  #ifdef DETAILED_MICROBENCHMARKER
    time_report(PO_EVICTION_END, level); 
    time_report(PO_UPLOAD_PATH_START, level); 
  #endif

  // printf("Printing path from LS before upload: \n");
  // printPath(leaf, level);

  //Encrypt and Upload Path :
  #ifdef EXITLESS_MODE
    *(req_struct->block) = false;			
  #else
    #ifndef PROTECT_STASH
      #ifdef ENCRYPTION_ON
        // unreachable("PathORAM_Access");
        encryptPath(decrypted_path, encrypted_path, (Z*(d)), tdata_size);		

        uploadPath(leaf, encrypted_path, path_size, level);				
      #else
        uploadPath(leaf, decrypted_path, path_size, level);  
      #endif
    #else
      // printf("TESTING1 : ID: %d, Label: %d\n", getId(decrypted_path), getTreeLabel(decrypted_path));
      // printPath_buffer(decrypted_path, d, data_size);
      // refreshnonce(decrypted_path, level);
      // printf("TESTING2 : ID: %d, Label: %d\n", getId(decrypted_path), getTreeLabel(decrypted_path));
      // printPath_buffer(decrypted_path, d, data_size);
      uploadPath(leaf, decrypted_path, path_size, level);
    #endif

  // printf("Path after uploadPath: \n");
  // showPath_BlockIDLabel(decrypted_path, Z, d, tdata_size, leaf);   

  // printf("Printing path from LS after upload: \n");
  // printPath(leaf, level);


    #ifdef DETAILED_MICROBENCHMARKER
      time_report(PO_UPLOAD_PATH_END, level); 
    #endif

    #ifdef SHOW_STASH_COUNT_DEBUG
      stash_oc = recursive_stash[level].stashOccupancy();
      printf("Level : %d , After upload: %d\n",level,stash_oc);		
    #endif

  #endif

  //printf("nextLeaf = %d",nextLeaf);
  return nextLeaf;
}

void PathORAM::PathORAM_RebuildPath(unsigned char* decrypted_path_ptr, uint32_t data_size, uint32_t block_size, uint32_t leaf, uint32_t level){
  uint32_t prefix;
  uint32_t i,k;
  uint32_t d = D_level[level];
  uint64_t n = N_level[level];
  unsigned char *decrypted_path_bucket_iterator = decrypted_path_ptr;
  unsigned char *decrypted_path_temp_iterator;
  unsigned char *refreshnonce_iterator;


  #ifdef PROTECT_STASH
    unsigned char* diffnonce = (unsigned char*)malloc(NONCE_LENGTH);
    uint32_t *nonceptr32 = (uint32_t*)diffnonce;
    for(int i = 0; i < NONCE_LENGTH/4; i++){
      nonceptr32[i] = rand();
    }
  #endif

  for(i=0;i<d;i++){
    prefix = ShiftBy(leaf+n,i);
    // printf("Prefix: %d\n", prefix);
    
    bool flag = false;
    nodev2 *listptr = NULL;
    listptr = recursive_stash[level].getStart();
    // printf("RebuildPath Stash leaf: %d\n", getTreeLabel(listptr->serialized_block));
    
    #ifdef PROTECT_STASH
      refreshnonce_iterator = decrypted_path_bucket_iterator;
      for(int l=0; l<Z; l++){
        o_refreshnonce(refreshnonce_iterator, diffnonce, data_size, isBlockDummy(refreshnonce_iterator, gN));
        refreshnonce_iterator += block_size;
      }
    #endif
      
    if(oblivious_flag) {
      uint32_t posk = 0;			
      for(k=0; k < stash_size; k++)
      {				
        decrypted_path_temp_iterator = decrypted_path_bucket_iterator;	
        // printf("RebuildPath: Shift: TreeLabel: %d, Treelabel+n: %d, i: %d", getTreeLabel(listptr->serialized_block), getTreeLabel(listptr->serialized_block)+n, i);		
        uint32_t jprefix = ShiftBy(getTreeLabel(listptr->serialized_block)+n,i);
        // printf("Jprefix: %d\n", jprefix);
        // if (i==0&&(!isBlockDummy(listptr->serialized_block, gN))){
        //   printf("leaf: %d, stash_id: %d stash_leaf: %d\n", leaf, getId(listptr->serialized_block), getTreeLabel(listptr->serialized_block));
        // }
        uint32_t sblock_written = false;
      
        bool flag = (posk<Z)&&(prefix==jprefix)&&(!sblock_written)&&(!isBlockDummy(listptr->serialized_block, gN));
        // if(prefix==jprefix){
        //   printf("posk: %d, prefix: %d, sblock: %d, dummy: %d\n", (posk<Z), (prefix==jprefix), (!sblock_written), (!isBlockDummy(listptr->serialized_block, gN)));
        // }
        for(uint8_t l=0;l<Z;l++){
          flag = (l==posk)&&(posk<Z) && (prefix==jprefix) && (!sblock_written) && (!isBlockDummy(listptr->serialized_block,gN));
        
          #ifdef PATHORAM_ACCESS_REBUILD_DEBUG
            if(flag){
              printf("Block %d,%d TO Bucket %d\n",getId(listptr->serialized_block),getTreeLabel(listptr->serialized_block),prefix);
            }
          #endif

          uint32_t tempgN = gN;

          #ifdef PROTECT_STASH
            uint32_t *nonce32 = (uint32_t*)(listptr->serialized_block);
            tempgN = gN^(*nonce32);
            // omove_buffer_NEW(decrypted_path_temp_iterator, listptr->serialized_block, data_size + ADDITIONAL_METADATA_SIZE, flag);
          #endif

          // omove_serialized_block(decrypted_path_temp_iterator, listptr->serialized_block, data_size, flag);
          omove_buffer_NEW(decrypted_path_temp_iterator, listptr->serialized_block, data_size + ADDITIONAL_METADATA_SIZE, flag);
          oset_value(getIdPtr(listptr->serialized_block), tempgN, flag);
          oset_value(&sblock_written, 1, flag);
          oincrement_value(&posk, flag);
          decrypted_path_temp_iterator+= block_size;
        }						
        listptr=listptr->next;
      }				
    }			
    else {	
      decrypted_path_temp_iterator = decrypted_path_bucket_iterator;			
      uint32_t posk = 0;
      nodev2 *listptr_prev = NULL, *listptr_prev2 = NULL;
      uint32_t cntr = 0;		

      while(listptr && posk<Z) { 						
        uint32_t jprefix = ShiftBy(getTreeLabel(listptr->serialized_block)+n, i);	
        bool flag = (prefix==jprefix);
        if(flag) {
          memcpy(decrypted_path_temp_iterator, listptr->serialized_block, block_size);	
          recursive_stash[level].remove(listptr,listptr_prev);
          posk++;
          decrypted_path_temp_iterator+= block_size;					
        }					
        if(!flag) {
          listptr_prev2 = listptr_prev;
          listptr_prev = listptr;	
          listptr = listptr->next;
        }				
      }

    }	
  
    /*
    #ifdef ACCESS_DEBUG
      decrypted_path_temp_iterator = decrypted_path_bucket_iterator;
      printf("rearrange : Block contents after oblock_move :\n");
      for(uint8_t e =0;e<Z;e++) {
        printf("(%d,%d) , ",getId(decrypted_path_temp_iterator),getTreeLabel(decrypted_path_temp_iterator));
        decrypted_path_temp_iterator+=block_size;
      }
      printf("\n");
    #endif
    */

    decrypted_path_bucket_iterator+=(Z*block_size);									
  }
  #ifdef PROTECT_STASH
    free(diffnonce);
  #endif
}



