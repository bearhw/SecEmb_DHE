import os
th = os.environ.get('CUSTOM_THREAD_COUNT') 
thread_count = "1" if th is None else th
print('thread_count', thread_count)
os.environ["OMP_NUM_THREADS"] = thread_count
os.environ["MKL_NUM_THREADS"] = thread_count
os.environ["OPENBLAS_NUM_THREADS"] = thread_count
os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
os.environ["NUMEXPR_NUM_THREADS"] = thread_count
threads = int(thread_count)

import sys
import json
import time
import csv
import  os
from contextlib import nullcontext
import random
import numpy as np
import argparse

import torch
from transformers import AutoTokenizer, AutoModel, AutoModelForCausalLM, LlamaForCausalLM

# torch.set_num_threads(int(threads))
# torch.set_num_interop_threads(int(threads))

out_dir = "RESULTS_OUTPUT/"



# Base configurations
def parse_configurations():
    parser = argparse.ArgumentParser(description='Inferece')
    parser.add_argument('--ext-type', type=str, default='undefined', help='Ext Type')    
    parser.add_argument('--batch-size', type=int, default=1, help='Batch Size')    
    return parser.parse_args()

args = parse_configurations()

BATCH = args.batch_size

emb_type = args.ext_type
print('EMBEDDING TYPE',emb_type)


VOCAB_SIZE = 50257

LENGTH_PREFILL = 256 
LENGTH_DECODE  = 128 

WARMUP = 2
RUNS = 10 * 2


device = "cpu"
# model dtype is f32


def initialize_model(model_name, **kwargs):              

    tokenizer = AutoTokenizer.from_pretrained(model_name)   
    model = AutoModelForCausalLM.from_pretrained(model_name, **kwargs)
    
    # tokenizer.pad_token = tokenizer.eos_token 
    # model.pad_token = tokenizer.eos_token 

    return tokenizer, model

 
def execute_gen(model, tokenizer, out_len):

    inputs = {}

    inputs['input_ids'] = torch.randint(0, VOCAB_SIZE, (BATCH, LENGTH_PREFILL))
    inputs['attention_mask'] =  torch.ones((BATCH, LENGTH_PREFILL))
                                 
    inference_start = time.time()
    generated_output = model.generate(**inputs, max_new_tokens=out_len, min_new_tokens=out_len, pad_token_id=tokenizer.eos_token_id)
    inference_end = time.time()

    # decoded_string = tokenizer.decode(generated_output[0]) # not valid unless valid weights

    return inference_end - inference_start


def main():
           
    model_name = "mohdumar-gpt2-medium-untied"

    if emb_type == 'index':        
        t, m = initialize_model(model_name)      
    elif emb_type == 'dhe':
        t, m = initialize_model(model_name, DHE_enabled=True, DHE_sizes="2048-2048-2048-2048" ) # dheK-MLP-MLP-MLP
    elif emb_type == 'ls':
        t, m = initialize_model(model_name, LS_enabled=True)        
    elif emb_type == 'ztpo':
        t, m = initialize_model(model_name, ZT_enabled=True, ZT_type="Path")
    elif emb_type == 'ztco':
        t, m = initialize_model(model_name, ZT_enabled=True, ZT_type="Circuit")
    else:
        panic()


    out_path = f"{out_dir}/output_inference_{emb_type}_threads{threads}_batch_{BATCH}.csv"
    output_data_file = open(out_path, 'w', newline='')
    writer = csv.writer(output_data_file)
    csv_row_titles = ["Threads", "RunIter", "BatchSize", "PrefillTokens", "DecodeNewTokens", "PrefillTime",  "PrefillDecodeTime", "PerTokenDecodeTime"] # header
    writer.writerow(csv_row_titles)

 
    
    for r in range(WARMUP):
        execute_gen(m, t, out_len=5)
        sys.stdout.flush()  

    run_i = 0
    
    for r in range(RUNS):

        # get prefill
        t_prefill = execute_gen(m, t, out_len=1)

        # get prefill+decode
        t_prefill_decode = execute_gen(m, t, out_len=LENGTH_DECODE)               

        # write results
        per_tok_decode_time = (t_prefill_decode - t_prefill)/LENGTH_DECODE
        writer.writerow([threads, run_i, BATCH, LENGTH_PREFILL, LENGTH_DECODE, round(t_prefill*1000,6), round(t_prefill_decode*1000,6), round(per_tok_decode_time*1000,6)  ])
        output_data_file.flush()

        run_i += 1
        sys.stdout.flush()

    output_data_file.close()



if __name__ == "__main__":
    
    main()

