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

#include "Stash.hpp"

//Oblivious Functions needed : 
/*
1) omove_buffer()
2)

*/

//Other Functions needed :
/*
1) getID()
2) getDataPtr(), isBlockDummy, getLabel, 
3) ^ All of those permuting functions ! 

*/
Stash::Stash(){
}

Stash::Stash(uint32_t param_stash_data_size, uint32_t param_STASH_SIZE, uint32_t param_gN){
	stash_data_size = param_stash_data_size;
	STASH_SIZE = param_STASH_SIZE;
	gN = param_gN;
}

void Stash::setParams(uint32_t param_stash_data_size, uint32_t param_STASH_SIZE, uint32_t param_gN){
	stash_data_size = param_stash_data_size;
	STASH_SIZE = param_STASH_SIZE;
	gN = param_gN;
}

void Stash::xordata64(unsigned char* data, unsigned char* nonce, uint32_t data_size, bool blockdata_flag/*=0*/){

	uint64_t *data64 = (uint64_t*)data;
	uint64_t *nonce64 = (uint64_t*)nonce;
	int i = 0;
	uint32_t size = 0;
	bool nonce_idx=0;

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

void Stash::PerformAccessOperation2(char opType, uint32_t id, uint32_t position_in_id, uint32_t newleaf, uint32_t newleaf_nextlevel, uint32_t* nextLeaf, uint32_t recursion_stash_size) {
	struct nodev2 *iter = getStart();
	struct nodev2 *temp2 = iter;
	uint8_t cntr = 1;
	uint32_t flag_id = 0, flag_w = 0, flag_r = 0;
	unsigned char *data_ptr;
	uint32_t *leaflabel_ptr;
	uint32_t *id_ptr;

	unsigned char *curr_nonce;
	unsigned char *diff_nonce;
	diff_nonce = (unsigned char*)malloc(NONCE_LENGTH);

	uint32_t *diff_nonce32 = (uint32_t*)diff_nonce;
	for(int i=0; i<NONCE_LENGTH/4; i++){
		diff_nonce32[i] = rand();
	}

	// printf("Here\n");
	int cnt = 0;
	

	while(iter&&cntr<=STASH_SIZE)	{

		
		
		curr_nonce = iter->serialized_block;
		id_ptr = getIdPtr(iter->serialized_block);
		data_ptr = (unsigned char*) getDataPtr(iter->serialized_block);
		leaflabel_ptr = getTreeLabelPtr(iter->serialized_block);

		flag_id = ( getId(iter->serialized_block) == id);

		uint32_t newid = (*id_ptr)^(*diff_nonce32);
		*id_ptr = newid;

		uint32_t* curr_nonce32 = (uint32_t*) curr_nonce;
		uint32_t temp_newleaf = (newleaf)^(*((uint32_t*)(curr_nonce+4)))^(*((uint32_t*)(diff_nonce+4)));

		*leaflabel_ptr = (*leaflabel_ptr)^(*((uint32_t*)(diff_nonce+4)));
		*leaflabel_ptr = cmov(flag_id, temp_newleaf, *leaflabel_ptr);

		// flag_w = (flag_id && opType == 'w');
		// flag_r = (flag_id && opType == 'r');

		// oaccess_stash_data(data_out2, data_ptr, curr_nonce, stash_data_size, flag_id);



		//Decrypt entire data and re encrypt after replacing tree. 
		
		

		// int res = -1;

		uint32_t* dataptr2 = (uint32_t*)data_ptr;
		xordata64(data_ptr, curr_nonce, recursion_stash_size, 1);
		*nextLeaf = cmov(flag_id, (dataptr2[position_in_id]), *nextLeaf);
		dataptr2[position_in_id] = cmov(flag_id, newleaf_nextlevel, dataptr2[position_in_id]);
		xordata64(data_ptr, curr_nonce, recursion_stash_size, 1);




		// if(flag_id){

		// 	uint32_t* dataptr2 = (uint32_t*)data_ptr;
		// 	// printf("CNT: \n", cnt);
		// 	// cnt++;
		// 	// printf("Pos in ID: %d, Val: %d\n", position_in_id, data2_32[position_in_id]);
		// 	xordata64(data_ptr, curr_nonce, 64, 1);
		// 	*nextLeaf = (dataptr2[position_in_id]);
		// 	dataptr2[position_in_id] = newleaf_nextlevel;
		// 	xordata64(data_ptr, curr_nonce, 64, 1);
		// 	// printf("NextLeaf: %d\n", *nextLeaf);
		// }

		// flag_r = (flag_id && opType == 'r');
		// if(flag_r){
		// 	printf("After PerformAccess, datasize = %d, Fetched Data :", stash_data_size);
		// 	for(uint32_t j=0; j < 16;j++){
		// 		printf("%d", data_out2[j]);
		// 	}
		// }

		// oaccess_stash_data(data_ptr, data_in, curr_nonce, stash_data_size, flag_w);
		
		xordata64(data_ptr, diff_nonce, 64, 1);
		xordata64(curr_nonce, diff_nonce, NONCE_LENGTH);


		iter = iter->next;
		cntr++;
	}   

	free(diff_nonce);

}
    
void Stash::PerformAccessOperation(char opType, uint32_t id, uint32_t newleaf, unsigned char *data_in, unsigned char *data_out) {
	// printf("In PerformAccessOperation\n");
	struct nodev2 *iter = getStart();
	struct nodev2 *temp2 = iter;
	uint8_t cntr = 1;
	uint32_t flag_id = 0, flag_w = 0, flag_r = 0;
	unsigned char *data_ptr;
	uint32_t *leaflabel_ptr;
	uint32_t *id_ptr;

	unsigned char *curr_nonce;
	unsigned char *diff_nonce;

	uint32_t* data_in32 = (uint32_t*)(data_in);

	#ifdef PROTECT_STASH
		diff_nonce = (unsigned char*)malloc(NONCE_LENGTH);

		uint32_t *diff_nonce32 = (uint32_t*)diff_nonce;
		for(int i=0; i<NONCE_LENGTH/4; i++){
			diff_nonce32[i] = rand();
		}

	#endif


	while(iter&&cntr<=STASH_SIZE)	{
		id_ptr = getIdPtr(iter->serialized_block);
		curr_nonce = iter->serialized_block;
		data_ptr = (unsigned char*) getDataPtr(iter->serialized_block);
		leaflabel_ptr = getTreeLabelPtr(iter->serialized_block);

		flag_id = ( getId(iter->serialized_block) == id);
		uint32_t temp_newleaf = newleaf;

		#ifdef PROTECT_STASH
			uint32_t* curr_nonce32 = (uint32_t*) curr_nonce;
			temp_newleaf = (newleaf)^(*((uint32_t*)(curr_nonce+4)))^(*((uint32_t*)(diff_nonce+4)));

			uint32_t newid = (*id_ptr)^(*diff_nonce32);
			*id_ptr = newid;

			*leaflabel_ptr = (*leaflabel_ptr)^(*((uint32_t*)(diff_nonce+4)));
			*leaflabel_ptr = cmov(flag_id, temp_newleaf, *leaflabel_ptr);


		#else
			//Replace leaflabel in block with newleaf
			oassign_newlabel(leaflabel_ptr, newleaf, flag_id);
		#endif


		#ifdef PAO_DEBUG
			if(flag_id == true){
				flag_found = true;
				printf("Found, has data: \n");
				unsigned char* data_ptr = getDataPtr(iter->serialized_block);
				for(uint32_t j=0; j < stash_data_size;j++){
				    printf("%c", data_ptr[j]);
				}	
				//setTreeLabel(iter->serialized_block,newleaf);
				printf("\n");
			}
		#endif

		flag_r = (flag_id && opType == 'r');
		#ifdef PAO_DEBUG
			if(flag_found){
				printf("flag_id = %d, optype=%c, flag_r = %d\n", flag_id, opType, flag_r);
			}
		#endif

		#ifdef PROTECT_STASH
			oaccess_stash_data(data_out, data_ptr, curr_nonce, stash_data_size, flag_r);
		#else

			#ifdef INLINE_ASM
				omove_buffer_NEW(data_out, (unsigned char*) data_ptr, stash_data_size, flag_r);
			#else
				omove_buffer(data_out, (unsigned char*) data_ptr, stash_data_size, flag_r);
			#endif
		#endif


		flag_w = (flag_id && opType == 'w');
		#ifdef PROTECT_STASH
			oaccess_stash_data(data_ptr, data_in, curr_nonce, stash_data_size, flag_w);
		#else

			#ifdef INLINE_ASM
				omove_buffer_NEW((unsigned char*) data_ptr, data_in, stash_data_size, flag_w);
			#else
				omove_buffer((unsigned char*) data_ptr, data_in, stash_data_size, flag_w);
			#endif
		#endif


		#ifdef PROTECT_STASH
			xordata64(data_ptr, diff_nonce, stash_data_size, 1);
			xordata64(curr_nonce, diff_nonce, NONCE_LENGTH);
		#endif


		iter = iter->next;
		cntr++;
	}
	#ifdef RESULTS_DEBUG
	printf("After PerformAccess, datasize = %d, Fetched Data :", stash_data_size);
	for(uint32_t j=0; j < stash_data_size;j++){
	    printf("%c", data_out[j]);
	}
	printf("\n");
	#endif
	#ifdef PAO_DEBUG
	if(flag_found == false){
	    printf("BLOCK NOT FOUND IN STASH\n");
	}
	#endif    

	#ifdef PROTECT_STASH
		free(diff_nonce);
	#endif  
	// printf("Done with PerformAccessOperation\n"); 

}
    
void Stash::ObliviousFillResultData(uint32_t id, unsigned char *result_data) {
    struct nodev2 *iter = getStart();
    uint8_t cntr = 1;
    uint32_t flag = 0;
    uint32_t *data_ptr;
    while(iter&&cntr<=STASH_SIZE)	{
        flag = ( getId(iter->serialized_block) == id );
        data_ptr = (uint32_t*) getDataPtr(iter->serialized_block);
		#ifdef INLINE_ASM
        	omove_buffer_NEW(result_data, (unsigned char*) data_ptr, stash_data_size, flag);
		#else
        	omove_buffer(result_data, (unsigned char*) data_ptr, stash_data_size, flag);
		#endif
        iter = iter->next;
        cntr++;
    }
}
   	
uint32_t Stash::displayStashContents(uint64_t nlevel, bool recursive_block) {
  uint32_t count = 0,cntr=1;
  nodev2 *iter = getStart();
  printf("Stash Contents : \n");
  while(iter&&cntr<=STASH_SIZE) {
    unsigned char *tmp;
    if( (!isBlockDummy(iter->serialized_block, gN)) ) {
      printf("loc = %d, (%d,%d) : ",cntr, getId(iter->serialized_block),getTreeLabel(iter->serialized_block));         
      tmp = iter->serialized_block + 24;
      uint32_t pbuckets = getTreeLabel(iter->serialized_block);
      count++;
      while(pbuckets>=1) {
          printf("%d, ", pbuckets);
          pbuckets = pbuckets>>1;
      }
      printf("\n");
      printf("Data: ");
      if(recursive_block){
        uint32_t *data_ptr = (uint32_t *) tmp;
        for(uint32_t j = 0; j<stash_data_size/(sizeof(uint32_t)); j++){
          printf("%d,", *data_ptr);
          data_ptr++; 
       }
     } 
      else{
        unsigned char *data_ptr = tmp;
        for(uint32_t j=0; j<stash_data_size; j++){
          printf("%c", data_ptr[j]);
        }
      }
      printf("\n");       
    }
    iter = iter->next;
    cntr++;
  }
  printf("\n");
  return count;
}

	
uint32_t Stash::stashOccupancy() {
    uint32_t count = 0,cntr=1;
    nodev2 *iter = getStart();
    while(iter&&cntr<=STASH_SIZE)	{
        if( (!isBlockDummy(iter->serialized_block, gN)) ) {
            count++;
        }
        iter = iter->next;
        cntr++;
    }
    return count;
}

		/*
		saveStash Format of serialized stash :
		<DataSize>
		<ID><Label><Data>
		<ID><Label><Data>
		<ID> = 4 bytes, <Label> = 4 bytes , <Data> = DataSize
		NOTE : No \n's in between.
		*/
		/*
		uint32_t saveStash(unsigned char *stash_serialized, uint32_t level)
		{
			if(start==NULL)	{
				return 0;
			}
			else{
				uint32_t datasize_ss;
				if(level==-1){
					datasize_ss = g_block_size;
				}
				else if(level==recursion_levels){
					datasize_ss = g_block_size;
				}	
				else{
					datasize_ss = recursion_block_size;
				}
				node *iter = start;
				unsigned char *ptr = stash_serialized;
				memcpy(ptr,&datasize_ss,4);
				ptr+=4;
				
				uint32_t i=0;
				while(iter) {
					//Edl buffer direction needs to be fixed.
					if(!(iter->block->isDummy())&&iter->occupied) {
						memcpy(ptr,&(iter->block->id),4);
						ptr+=4;
						memcpy(stash_serialized,&(iter->block->treeLabel),4);						
						ptr+=4;						
						memcpy(stash_serialized,iter->block->data, datasize_ss);
						ptr+=datasize_ss;
					}
					iter=iter->next;
					i++;
				}
				uint32_t STASH_SIZE_total = 4 + ((datasize_ss+8) * i);
				return STASH_SIZE_total;
			}
		}
		void restoreStash(uint32_t *stash_serialized,uint32_t STASH_SIZE)
		{
			node *iter = start;
			uint32_t i= 0;
			while(STASH_SIZE!=0)
			{
				iter->block->id = stash_serialized[i];
				i++;
				iter->block->treeLabel = stash_serialized[i];
				i++;
				iter->occupied = true;
				iter = iter->next;
				STASH_SIZE--;
			}
		}
		*/

struct nodev2* Stash::getStart(){
	return start;
}

void Stash::setStart(struct nodev2* new_start){
	start = new_start;
}

void Stash::setup(uint32_t pstash_size, uint32_t pdata_size, uint32_t pgN)
{	
	gN = pgN;
	STASH_SIZE = pstash_size;
	printf("Stash data size: %d\n", pdata_size);
	stash_data_size = pdata_size;
        current_size=0;
	for(uint32_t i = 0; i<STASH_SIZE; i++){
		insert_new_block();
	}
}

void Stash::setup_nonoblivious(uint32_t pdata_size, uint32_t pgN)
{	
	gN = pgN;
	stash_data_size = pdata_size;
}


void Stash::insert_new_block()
{
  Block block(stash_data_size, gN);
	struct nodev2 *new_node = (struct nodev2*) malloc(sizeof(struct nodev2));			

	if(current_size == STASH_SIZE){
		printf("%d, Stash Overflow here\n", current_size);
	}
	else{
		#ifdef PROTECT_STASH
			unsigned char *serialized_block = block.serialize(stash_data_size, 1);
		#else
			unsigned char *serialized_block = block.serialize(stash_data_size);
		#endif
		new_node->serialized_block = serialized_block;
		new_node->next = getStart();
		setStart(new_node);	
    	// printf("Inserted block (%d, %d) into stash\n", getId(serialized_block), getTreeLabel(serialized_block));
		current_size+=1;
	}
}

void Stash::remove(nodev2 *ptr, nodev2 *prev_ptr)
{			
    nodev2 *temp;
    if(prev_ptr==NULL&&current_size==1) {
        start=NULL;
    }
    else if(prev_ptr==NULL) {
        start = ptr->next;			
    }
    else if(ptr->next==NULL) {
        prev_ptr->next = NULL;
    }
    else {
        prev_ptr->next = ptr->next;
    }

    free(ptr->serialized_block);
    free(ptr);
    current_size--;

}

void Stash::pass_insert(unsigned char *serialized_block, bool is_dummy)
{
    struct nodev2 *iter = start;
    bool block_written = false;
    uint8_t cntr = 1;

	uint32_t stash_block_size = stash_data_size + ADDITIONAL_METADATA_SIZE;
    #ifdef STASH_OVERFLOW_DEBUG
        bool inserted = false;
    #endif
    while(iter&&cntr<=STASH_SIZE)	{
        bool flag = (!is_dummy && (isBlockDummy(iter->serialized_block, gN)) && !block_written);

        #ifdef STASH_OVERFLOW_DEBUG
            inserted = inserted || flag;
        #endif
		#ifdef PROTECT_STASH
			block_written = cmov(flag, 1, block_written);
			omove_buffer_NEW(iter->serialized_block, serialized_block, stash_block_size, flag);
		#else
        	stash_serialized_insert(iter->serialized_block, serialized_block, stash_data_size, flag, &block_written);
			
		#endif
        iter = iter->next;
        cntr++;
    }
    #ifdef STASH_OVERFLOW_DEBUG
        if(!is_dummy && !inserted){
            printf("STASH OVERFLOW \n");
        }
    #endif

}		

void Stash::insert( unsigned char *serialized_block)
{
    struct nodev2 *new_node = (struct nodev2*) malloc(sizeof(struct nodev2));			
    struct nodev2 *temp_start = getStart();

    if(current_size == STASH_SIZE){
        printf("Stash Overflow \n");
    }
    else{
        new_node->serialized_block = serialized_block;
        new_node->next = getStart();
        setStart(new_node);	
        current_size+=1;
    }
}
