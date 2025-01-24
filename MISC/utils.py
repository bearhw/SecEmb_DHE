
import math


# MB_factor = 1e6
MB_factor = 1048576




def parse_dims(dims):
    dims_parsed = [int(i) for i in dims.split('-')]
    return dims_parsed



def get_mlp_size(mlp_dims):    
    bytes_per_param = 4
    mlp_size = 0
    for i in range(len(mlp_dims) - 1):
        dim_in  = int(mlp_dims[i])
        dim_out = int(mlp_dims[i + 1])            
        mlp_size  += (dim_in + 1) * dim_out * bytes_per_param
    return mlp_size
    


def get_raw_table_memory(emb_dim,tableSizes_list):

    total_memory = 0
    for table_size in tableSizes_list:
        precision_bytes = 4 # fp32
        emb_bytes = emb_dim * precision_bytes
        total_memory += emb_bytes * table_size        
    return total_memory



# for dlrm
def get_dhe_uniform_memory(embSize, tableSizes_list):
    
    table_count = len(tableSizes_list)    
    ## common DHE Uniform parameters
    dheK = 1024  
    dhe_mlp_dims = str(dheK)+"-512-256-"+str(embSize)  

    # DHE Encoder
    bytes_per_param = 8
    dhe_encoder_size = 4 * dheK * table_count * bytes_per_param        # a/b/m/p per table
    # DHE Decoder
    dhe_decoder_size = get_mlp_size(parse_dims(dhe_mlp_dims)) * table_count
    return dhe_encoder_size + dhe_decoder_size


# for dlrm
def get_dhe_varied_memory(embSize, tableSizes_list):
    
    total_memory = 0
    for table_size in tableSizes_list:        
        z = math.floor(math.log10(table_size))
        y = 6 - z # 1e6 starting point    
        dheK = 1024-128*y
        dim1 = 512-64*y
        dim2 = 256-32*y         
        dhe_mlp_dims = str(dheK)+f"-{dim1}-{dim2}-"+str(embSize)    
        # DHE Encoder
        bytes_per_param = 8
        dhe_encoder_size = 4 * dheK * bytes_per_param        # a/b/m/p per table
        # DHE Decoder
        dhe_decoder_size = get_mlp_size(parse_dims(dhe_mlp_dims))
        total_memory += dhe_encoder_size + dhe_decoder_size
    return total_memory


# dlrm
def split_list(curr_threshold, tableSizes_list):
    list1 = [] # LS
    list2 = [] # DHE
    for table_size in tableSizes_list:
        if table_size <= curr_threshold:
            list1.append(table_size)
        else:
            list2.append(table_size)
    return list1, list2




# PORAM
stash_size = 150
MEM_POSMAP_LIMIT = 4096 * 4     

# CORAM
MEM_POSMAP_LIMIT = 256 * 4     
stash_size = 10 


# Global for ORAM
recursion_block_size = 64             
x = int(recursion_block_size / 4)    # 16 in our experiments
emb_precision_bytes = 4 # fp32


def get_oram_size(data_size, max_blocks):
            
    pZ = 4 # bucket

    block_size = data_size + 4 + 4      # id and treeLabel

    D = math.ceil( math.log(max_blocks/pZ)/ math.log(2));
    N = math.pow(2, D)
    TreeDepth = D+1
    ptreeSize = 2*N-1
    tree_memory = ptreeSize*block_size*pZ
    # print(D, N, TreeDepth, ptreeSize,   N*pZ, block_size)

    posmap_memory = max_blocks * 4 # uint32

    stash_memory = stash_size * block_size

    return tree_memory, posmap_memory, stash_memory
  

def get_embedding_size_table_oram(emb_dim, tableSizes_list):

    total_memory_ORAM = 0

    for table_size in tableSizes_list:

        emb_bytes = emb_dim * emb_precision_bytes
      
        max_blocks = table_size

        size_pmap0 = max_blocks * 4 # in bytes, ID is 4 bytes
        cur_pmap0_blocks = max_blocks        
        recursion_levels = 1    
        while size_pmap0 > MEM_POSMAP_LIMIT:        
            cur_pmap0_blocks = math.ceil(cur_pmap0_blocks / x)
            size_pmap0 = cur_pmap0_blocks * 4;
            recursion_levels += 1            
        # print(cur_pmap0_blocks, size_pmap0, 'recursion_levels', recursion_levels)              
        
        blocks_in_level = [ ]
        last = cur_pmap0_blocks
        blocks_in_level.append(last)
        for lvl in range(recursion_levels-1):
            last = last * x
            blocks_in_level.append(last)
        # print('blocks_in_level', blocks_in_level)
        
        posmap_memory = blocks_in_level[0] * 4 # posmap only initial level only   

        if recursion_levels > 1:
            blocks_in_level[0] = blocks_in_level[1]  
                    
            for level_blocks in blocks_in_level[1:len(blocks_in_level)-1]:          # middle levels ignoring last
                sz_oram_tree, _ , sz_stash = get_oram_size(recursion_block_size, level_blocks) # 
                total_memory_ORAM += sz_oram_tree + sz_stash
                # print('Posmap Tree',level_blocks,'   ',sz_oram_tree, sz_stash)
        
            for level_blocks in [blocks_in_level[len(blocks_in_level)-1]]:          # last level
                sz_oram_tree, _ , sz_stash = get_oram_size(emb_dim * emb_precision_bytes, level_blocks)                    
                total_memory_ORAM += sz_oram_tree + sz_stash
                # print('Data Tree',level_blocks,'   ',sz_oram_tree, sz_stash)
                
        else: # 1  
            sz_oram_tree, _ , sz_stash = get_oram_size(emb_dim * emb_precision_bytes, blocks_in_level[0])                    
            total_memory_ORAM += sz_oram_tree + sz_stash
            # print('Data Tree',blocks_in_level[0],'   ',sz_oram_tree, sz_stash)                       
             
        total_memory_ORAM += posmap_memory
        
        # print('\n')

    return total_memory_ORAM


