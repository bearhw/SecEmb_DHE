
from utils import *


#### LLM

model_name = 'GPT-2 Medium'
gpt2_medium_param_count = 354823168
table_count = 1
tableSize = 50257
embSize = 1024 

bytes_per_param = 4
gpt2_medium_model_size = gpt2_medium_param_count * bytes_per_param

emb_table_size = tableSize * embSize * bytes_per_param
# still table weights exist in last LM head


## common DHE Uniform parameters
dheK = 2048  
dhe_mlp_dims = str(dheK)+"-"+str(dheK)+"-"+str(dheK)+"-"+str(dheK)+"-"+str(embSize)


## DHE sizes
# DHE Encoder
bytes_per_param = 8
dhe_encoder_size = 4 * dheK * table_count * bytes_per_param        # a/b/m/p
# DHE Decoder
dhe_decoder_size = get_mlp_size(parse_dims(dhe_mlp_dims)) * table_count


emb_table_oram_size = get_embedding_size_table_oram(embSize, [tableSize])




print('Model:',model_name)
print('Original Model Size:',(gpt2_medium_model_size)/MB_factor,' MB')
print('Original Table Size:',(emb_table_size)/MB_factor,' MB')
print('DHE Layer Size:',(dhe_encoder_size+dhe_decoder_size)/MB_factor,' MB')
print('Table-ORAM Size:',(emb_table_oram_size)/MB_factor,' MB')



