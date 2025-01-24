
from utils import *

#### DLRM

table_count = 26

##### choose one:

### kaggle
model_name = 'Kaggle'
embSize = 16 
tableSizes_list = '3-4-10-15-18-24-27-105-305-583-633-1460-2173-3194-5652-5683-12517-14992-93145-142572-286181-2202608-5461306-7046547-8351593-10131227'
mlp_bot_dims_str = "13-512-256-64-"+str(embSize)   
mlp_top_dims_str = str((table_count+1)*embSize)+"-512-256-1"
threshold_unif = 14991  #  32B 1Thr
threshold_vary = 10000 # 


### terabyte
model_name = 'Terabyte'
embSize = 64 
tableSizes_list = '3-4-10-14-36-61-101-122-970-1442-2208-7112-7378-11156-12420-17217-20134-36084-313829-415421-1333352-7267859-9758201-9946608-9980333-9994222'
mlp_bot_dims_str = "13-512-256-"+str(embSize)  
mlp_top_dims_str = str((table_count+1)*embSize)+"-512-512-256-1"
threshold_unif = 5000 # 
threshold_vary = 2000 # 



 
 
 
 
 
 
 
## Bottom/Top MLP sizes    
mlp_bot_size = get_mlp_size(parse_dims(mlp_bot_dims_str))     
mlp_top_size = get_mlp_size(parse_dims(mlp_top_dims_str))     



## Embedding Table sizes
total_emb_table_size = get_raw_table_memory(embSize, parse_dims(tableSizes_list))
total_emb_table_oram_size = get_embedding_size_table_oram(embSize, parse_dims(tableSizes_list))

print('Model:',model_name)
print('Table Model Size:',(mlp_bot_size+mlp_top_size+total_emb_table_size)/MB_factor,' MB')
print('Table-ORAM Model Size:',(mlp_bot_size+mlp_top_size+total_emb_table_oram_size)/MB_factor,' MB')


## DHE & Hybrid sizes

sz_dhe_unif = get_dhe_uniform_memory(embSize, parse_dims(tableSizes_list))
sz_dhe_vary = get_dhe_varied_memory(embSize, parse_dims(tableSizes_list))
 
print('DHE-Uniform Model Size:',(mlp_bot_size+mlp_top_size+sz_dhe_unif)/MB_factor,' MB')
print('DHE-Varied Model Size:',(mlp_bot_size+mlp_top_size+sz_dhe_vary)/MB_factor,' MB')
 
 


list_ls, list_dhe = split_list(threshold_unif, parse_dims(tableSizes_list))
sz_dhe_part = get_dhe_uniform_memory(embSize, list_dhe)
sz_ls_part = get_raw_table_memory(embSize, list_ls)
print('Hybrid-Uniform Model Size:',(mlp_bot_size+mlp_top_size+sz_dhe_part+sz_ls_part)/MB_factor,' MB')


list_ls, list_dhe = split_list(threshold_vary, parse_dims(tableSizes_list))
sz_dhe_part = get_dhe_varied_memory(embSize, list_dhe)
sz_ls_part = get_raw_table_memory(embSize, list_ls)
print('Hybrid-Varied Model Size:',(mlp_bot_size+mlp_top_size+sz_dhe_part+sz_ls_part)/MB_factor,' MB')


