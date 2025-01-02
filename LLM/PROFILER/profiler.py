import os
th = os.environ.get('CUSTOM_THREAD_COUNT') 
thread_count = "1" if th is None else th
print('thread_count', thread_count)
os.environ["OMP_NUM_THREADS"] = thread_count
os.environ["MKL_NUM_THREADS"] = thread_count
os.environ["OPENBLAS_NUM_THREADS"] = thread_count
os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
os.environ["NUMEXPR_NUM_THREADS"] = thread_count


import argparse
import numpy as np
import random
import numpy as np
import sys
from time import time
import torch.nn as nn
import torch
import csv



out_dir = "RESULTS_OUTPUT/"




# Base configurations
def parse_configurations():
    parser = argparse.ArgumentParser(description='Profiler')
    parser.add_argument('--ext-type', type=str, default='undefined', help='Ext Type')    
    parser.add_argument('--embTableSize', type=str, default='32x64', help='')
    return parser.parse_args()

args = parse_configurations()

##### NLP
print('args.embTableSize', args.embTableSize)
table_cfg = args.embTableSize.split('x')

embSize =  int(table_cfg[1]) 
tableSize = int(table_cfg[0])
print('ARG TABLE',tableSize,embSize)




## LLM inference level batch sizes
# batchSize_list = [ 1,4,8,16 ] # 16==4096 (256 prefill * 16) effective batch size in prefill
batchSize_list = [ 1,8 ]
# batchSize_list = [ 16 ]



emb_type = args.ext_type
print('EMB TYPE',emb_type)


 

from emb_wrappers import DHE_Emb
from emb_wrappers import LS_Emb
from emb_wrappers import ZT_ORAM_Emb







times_embed = []

class Test_Model(nn.Module):
    
    def __init__(self, vocab_size, embed_dim):
        super(Test_Model, self).__init__()            

        if emb_type == 'index':
            self.wte = torch.nn.Embedding(vocab_size, embed_dim) 
        elif emb_type == 'ls':
            self.wte = LS_Emb(vocab_size, embed_dim) 
        elif emb_type == 'dhe':
            k = str(embed_dim*2)
            self.wte = DHE_Emb(embed_dim, f"{k}-{k}-{k}-{k}" )        # k and 3mlp are twice dim      # e.g. k-mlp-mlp-mlp "2048-2048-2048-2048" for dim 1024
        elif emb_type == 'ztpo':
            self.wte = ZT_ORAM_Emb(vocab_size, embed_dim, "Path") 
        elif emb_type == 'ztco':
            self.wte = ZT_ORAM_Emb(vocab_size, embed_dim, "Circuit") 
        else:
            panic()
        
        # noise the measurement wrt cache   
        m = 1280
        n = 2560
        k = 1920
        self.tensor1 = torch.randn(10, m, n)
        self.tensor2 = torch.randn(10, n, k)


    def forward(self, x):

        # noise the measurement wrt cache           
        if True:    
            _ = torch.matmul(self.tensor1, self.tensor2)
        
        global times_embed
        
        t0 = time()
        out = self.wte(x)
        t1 = time()

        times_embed.append(round(1000*(t1 - t0),3))
                        
        return out
    
    
test_model =     Test_Model(tableSize,embSize)

test_model.eval() # inference




TOKENS_DECODE  = 1              
TOKENS_PREFILL = 256
TOKENS_LIST = [TOKENS_DECODE, TOKENS_PREFILL]



RUNS = 32

VOCAB_SIZE = 50257





import csv
thr = 1 if emb_type=='ztpo' or emb_type=='ztco' else int(thread_count)
out_path = f"{out_dir}/output_{emb_type}_{args.embTableSize}_threads_{thr}.csv"
output_data_file = open(out_path, 'w', newline='')
writer = csv.writer(output_data_file)
# csv_row_titles = ["Rows","Cols","BatchMain","BatchTokens","Lat"]
csv_row_titles = ["Threads","Rows","EmbSize","BatchMain","BatchTokens","BatchEffective","TimeEmb"]
writer.writerow(csv_row_titles)


    
for batchSize in batchSize_list:            

    for tokens in TOKENS_LIST:
        
        print('effective-batch', batchSize, 'x', tokens)
    
        times_embed = []
    
        for r in range(RUNS):
                            
            x_in = torch.randint(0, VOCAB_SIZE, (batchSize, tokens))
            
            test_model.forward(x_in)


        avglat = round(np.average(times_embed[1:]),4) # average except first    
        print('times_embed',avglat)
           
        writer.writerow([int(thread_count),tableSize,embSize,batchSize,tokens,batchSize*tokens,avglat]) 
        
        output_data_file.flush()

  
output_data_file.close()

