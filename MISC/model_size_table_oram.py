import math

### kaggle
# tableSizes_list = [3, 4, 10, 15, 18, 24, 27, 105, 305, 583, 633, 1460, 2173, 3194, 5652, 5683, 12517, 14992, 93145, 142572, 286181, 2202608, 5461306, 7046547, 8351593, 10131227]
# emb_dim = 16

###### terabyte
tableSizes_list = [3, 4, 10, 14, 36, 61, 101, 122, 970, 1442, 2208, 7112, 7378, 11156, 12420, 17217, 20134, 36084, 313829, 415421, 1333352, 7267859, 9758201, 9946608, 9980333, 9994222]
emb_dim = 64

##### gpt2 med
# tableSizes_list = [50257]
# emb_dim = 1024


# no recursion


total_memory = 0
total_memory_ORAM = 0

for table_size in tableSizes_list:

    print(table_size)
    max_blocks = table_size
    
    pZ = 4 # bucket

    # precision_bytes = 1 # int8
    precision_bytes = 4 # fp32

    emb_bytes = emb_dim * precision_bytes
    block_size = emb_bytes + 4 + 4 # id and treeLabel

    D = math.ceil( math.log(max_blocks/pZ)/ math.log(2));
    N = math.pow(2, D)
    TreeDepth = D+1
    ptreeSize = 2*N-1
    print(D,N,ptreeSize,block_size)
    tree_memory = ptreeSize*block_size*pZ
    print('tree',tree_memory/1048576, 'MB')

    posmap_memory = max_blocks * 4 # uint32
    print('posmap',posmap_memory/1048576, 'MB')

    stash_size = 150
    stash_size = 10
    stash_memory = stash_size * block_size
    print('stash',stash_memory,'B')

    print('total',(tree_memory+posmap_memory+stash_memory)/1048576, 'MB')

    print('\n\n\n')

    total_memory += emb_bytes * table_size

    total_memory_ORAM += tree_memory + posmap_memory + stash_memory


# print('total_memory B', total_memory_ORAM)
print('total_memory MB \t\t' , total_memory/1048576)
print('total_memory_ORAM MB \t\t' , total_memory_ORAM/1048576)



