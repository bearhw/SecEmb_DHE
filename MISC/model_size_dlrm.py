
### Model Sizes

import math



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
    

def get_embedding_size_table_oram(emb_dim,tableSizes_list):

    total_memory = 0
    total_memory_ORAM = 0

    for table_size in tableSizes_list:

        max_blocks = table_size
        
        pZ = 4 # bucket

        precision_bytes = 4 # fp32

        emb_bytes = emb_dim * precision_bytes
        block_size = emb_bytes + 4 + 4 # id and treeLabel

        D = math.ceil( math.log(max_blocks/pZ)/ math.log(2));
        N = math.pow(2, D)
        TreeDepth = D+1
        ptreeSize = 2*N-1
        # print(D,N,ptreeSize,block_size)
        tree_memory = ptreeSize*block_size*pZ

        posmap_memory = max_blocks * 4 # uint32

        stash_size = 150
        stash_size = 10
        stash_memory = stash_size * block_size

        total_memory += emb_bytes * table_size

        total_memory_ORAM += tree_memory + posmap_memory + stash_memory

    return total_memory, total_memory_ORAM



MB_factor = 1e6
# MB_factor = 1048576




### DLRM

table_count = 26


# choose one:

### kaggle
model_name = 'Kaggle'
embSize = 16 
tableSizes_list = '3-4-10-15-18-24-27-105-305-583-633-1460-2173-3194-5652-5683-12517-14992-93145-142572-286181-2202608-5461306-7046547-8351593-10131227'
mlp_bot_dims_str = "13-512-256-64-"+str(embSize)   
mlp_top_dims_str = str((table_count+1)*embSize)+"-512-256-1"


### terabyte
# model_name = 'Terabyte'
# embSize = 64 
# tableSizes_list = '3-4-10-14-36-61-101-122-970-1442-2208-7112-7378-11156-12420-17217-20134-36084-313829-415421-1333352-7267859-9758201-9946608-9980333-9994222'
# mlp_bot_dims_str = "13-512-256-"+str(embSize)  
# mlp_top_dims_str = str((table_count+1)*embSize)+"-512-512-256-1"






## common DHE Uniform parameters
dheK = 1024  
dhe_mlp_dims = str(dheK)+"-512-256-"+str(embSize)




## Bottom/Top MLP sizes    
mlp_bot_size = get_mlp_size(parse_dims(mlp_bot_dims_str))     
mlp_top_size = get_mlp_size(parse_dims(mlp_top_dims_str))     


## DHE sizes
# DHE Encoder
bytes_per_param = 8
dhe_encoder_size = 4 * dheK * table_count * bytes_per_param        # a/b/m/p per table
# DHE Decoder
dhe_decoder_size = get_mlp_size(parse_dims(dhe_mlp_dims)) * table_count


## Embedding Table sizes
total_emb_table_size, total_emb_table_oram_size = get_embedding_size_table_oram(embSize, parse_dims(tableSizes_list))




print('Model:',model_name)
print('Table Model Size:',(mlp_bot_size+mlp_top_size+total_emb_table_size)/MB_factor,' MB')
print('DHE-Uniform Model Size:',(mlp_bot_size+mlp_top_size+dhe_encoder_size+dhe_decoder_size)/MB_factor,' MB')
print('Table-ORAM Model Size:',(mlp_bot_size+mlp_top_size+total_emb_table_oram_size)/MB_factor,' MB')


