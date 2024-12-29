
########## LLM
# model + DHE + Table/Head

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
    
# MB_factor = 1e6
MB_factor = 1048576


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



print('Model:',model_name)
print('Original Model Size:',(gpt2_medium_model_size)/MB_factor,' MB')
print('Original Table Size:',(emb_table_size)/MB_factor,' MB')
print('DHE layer overhead:',(dhe_encoder_size+dhe_decoder_size)/MB_factor,' MB')

