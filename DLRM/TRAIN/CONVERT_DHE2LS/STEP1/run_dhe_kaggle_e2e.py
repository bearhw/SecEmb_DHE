import os

# print('DEBUG ', os.environ.get('OMP_NUM_THREADS')  )

th = os.environ.get('CUSTOM_THREAD_COUNT') 
thread_count = "1" if th is None else th

print('thread_count', thread_count)

os.environ["OMP_NUM_THREADS"] = thread_count
os.environ["MKL_NUM_THREADS"] = thread_count
os.environ["OPENBLAS_NUM_THREADS"] = thread_count
os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
os.environ["NUMEXPR_NUM_THREADS"] = thread_count

######################

import numpy as np

import sys
from time import time
import shared_vars

import torch

from gen_data import GeneratedDataset, make_dataloader
from utils import fix_seed, parse_configurations, parse_dims


def main(args):

    global result_str

    args.mlp_top_dims = parse_dims(args.mlp_top_dims)
    args.mlp_bot_dims = parse_dims(args.mlp_bot_dims)
    args.table_sizes  = parse_dims(args.table_sizes)

    print('-------------------- Enabling DHE --------------------')
    # DHE Parameters
    args_dhe = {}
    args_dhe['activation']  = args.dhe_activation
    args_dhe['batch_norm']  = args.dhe_batch_norm
    args_dhe['hash_fn']     = args.dhe_hash_fn
    args_dhe['k']           = args.dhe_k
    args_dhe['m']           = args.dhe_m
    args_dhe['mlp_dims']    = parse_dims(args.dhe_mlp_dims)
    args_dhe['num_lookups'] = args.num_lookups
    args_dhe['precision']   = args.precision
    args_dhe['seed']        = args.seed
    args_dhe['transform']   = args.dhe_transform
    args_dhe['dhe_varying'] = args.dhe_varying
    
    print('---------- DHE Parameters: ----------')
    for key, val in args_dhe.items():
        print('{} : {}'.format(key, val))
    print('--------------------')


    # Default: CPU, GPU
    if 1:
        from dhe import DHE
        if args.device == 'cpu':
            device = 'cpu'
            print('Using CPU...')
        elif args.device == 'gpu' and torch.cuda.is_available():
            device = 'cuda:0'
            print('Using GPU...')
        

    # Fix Seed
    fix_seed(args.seed)

    # Data Generation
    dataset = GeneratedDataset(
        num_batches = args.num_batches,
        batch_size  = args.batch_size,
        dense_dim   = args.dense_dim,
        table_sizes = args.table_sizes,
        num_lookups = args.num_lookups,
        precision   = args.precision
    )

    dataloader = make_dataloader(dataset)

    # Model and Optimizer Setup

    if args.device == 'cpu' or args.device == 'gpu':
        model = DHE(
            dense_dim    = args.dense_dim,
            mlp_bot_dims = args.mlp_bot_dims,
            mlp_top_dims = args.mlp_top_dims,
            emb_dim      = args.emb_dim,
            table_sizes  = args.table_sizes,
            num_lookups  = args.num_lookups,
            precision    = args.precision,
            args_dhe     = args_dhe
        )
        # model = torch.compile(model) # <- magic happens!
        # model = torch.compile(model, mode='reduce-overhead') 

        model.to(device)
        # Create Optimizer
        optimizer = torch.optim.SGD(model.parameters(), lr=args.lr)


    print(model)
    
    
    
    
    
    print("Model's state_dict:")
    for param_tensor in model.state_dict():
        print(param_tensor, "\t", model.state_dict()[param_tensor].size())
    
    trained = torch.load('../../TRAINED_MODELS/kaggle_dhe_uniform.pt',map_location=torch.device('cpu'))
    
    # dictionary[new_key] = dictionary.pop(old_key)
    trained_dict = trained['state_dict']
    new_trained_dict = {}
    for key in trained_dict:
        new_key = ""
        if 'dhe_l' in key:
            new_key = key.replace('dhe_l','dhe_stacks')
        if 'bot_l' in key:
            new_key = key.replace('bot_l','mlp_bot')
        if 'top_l' in key:
            new_key = key.replace('top_l','mlp_top')
        new_trained_dict[new_key] = trained_dict[key]

    model.load_state_dict(new_trained_dict, strict=True)
    # new_trained_dict = None
    # trained = None
    model.eval()
                    
    print('\n\n\n\n')
    # val = 0
    # out = model.apply_dhe_stacks(torch.ones(26,1),torch.ones(26,1)*val)
        
    
    
    # list_of_output_lists.append(out)
        
    # exit()
    
    tableSizes_list = '1460-583-10131227-2202608-305-24-12517-633-3-93145-5683-8351593-3194-27-14992-5461306-10-5652-2173-4-7046547-18-15-286181-105-142572'
    table_ctr = 0
    
    for tableSize in tableSizes_list.split('-'):
        
        tableSize = int(tableSize)
        
        list_of_output_lists = []    
    
        BS = 80
        batch_ctr = 0
    
        # for val in range(1,333): # 
        
        # for val in range(0,tableSize): 
        while True:
            
            # no grad
            # out_new = model.apply_dhe_stacks(torch.ones(26,1),torch.ones(26,1)*val)
            
            batch_start = BS * batch_ctr
            batch_end = BS * (batch_ctr+1)
            batch_end = min(tableSize, batch_end)
            thisBS = batch_end-batch_start
            
            if batch_start >= tableSize:
                break
            # print(batch_start,batch_end,thisBS)
            
            
            indx = torch.arange(start=batch_start, end=batch_end).unsqueeze(0)
            # print('indx',indx.shape)
            
            with torch.no_grad(): 
                out_new = model.dhe_stacks.forward_individual(table_ctr, torch.ones(1,thisBS), indx) [0]
                # print('out_new',out_new.shape)
            
            list_of_output_lists.append(out_new)
            
            batch_ctr += 1

            
            
        # print('list_of_output_lists',len(list_of_output_lists))
        tnsr = torch.cat(list_of_output_lists,dim=0)#.squeeze()
        print(tableSize,'  cat',tnsr.shape)
        torch.save(tnsr, f"CACHED_TABLES/kaggle_tbl{table_ctr}.pt")
        table_ctr += 1
        
        
        # print('debug final-0',out[0].shape)
            
    exit()

    
    
    
    


if __name__ == '__main__':
    

    # Experiment Arguments
    args = parse_configurations()
    print('DEFAULT args ', args)

    # once only
    args.precision    = eval(args.precision)

    result_str = ""

    # warm-up run
    # main(args)    
    # reset 
    result_str = ""
    # shared_vars.times_bot_mlp = []
    # shared_vars.times_embed = []
    # shared_vars.times_interact = []
    # shared_vars.times_top_mlp = []
    # shared_vars.times_embed_dhe_hash = []
    # shared_vars.times_embed_dhe_mlp = []

    ###################

    batchSize_list = [ 1 ]
    
    denseFeatures = 13
    args.dense_dim = denseFeatures

    # adjust table size here as well if needed
    tableSizes_list=['100000'] # if 1 table size only
    print('tableSizes_list ', tableSizes_list)        
    print('')

    embSize_list = [ 16 ]

    dheK_list = [ 1024 ]

    numLookups_list = [ 1 ]


    mlp_bot_dims_str =  "13-512-256-64-"
    mlp_top_dims_str =  "512-256-1"



    # # kaggle
    embSize_list = [ 16 ]
    tableSizes_list = ['1460-583-10131227-2202608-305-24-12517-633-3-93145-5683-8351593-3194-27-14992-5461306-10-5652-2173-4-7046547-18-15-286181-105-142572']
    mlp_bot_dims_str =  "13-512-256-64-" #-16 
    mlp_top_dims_str =  "512-256-1"





    num_expr = 0


    for batchSize in batchSize_list:

        for tableSizes in tableSizes_list:

            for embSize in embSize_list:
                
                for numLookups in numLookups_list:
                         
                    for dheK in dheK_list:
                        
                        print('\n\n\n\n\n')
                        print('num_expr',num_expr)
                        sys.stdout.flush()

                        num_expr += 1

                        args.batch_size = batchSize                        

                        args.table_sizes = tableSizes
                        args.emb_dim = embSize
                        args.mlp_bot_dims = mlp_bot_dims_str+str(embSize)    
                        args.mlp_top_dims = mlp_top_dims_str
                        args.dhe_k = dheK
                        args.dhe_mlp_dims = str(dheK)+"-512-256-"+str(embSize)
                        args.num_lookups = numLookups
                        # print(args)

                        main(args)
                        
                        # reset after current experiment 
                        shared_vars.times_bot_mlp = []
                        shared_vars.times_embed = []
                        shared_vars.times_interact = []
                        shared_vars.times_top_mlp = []
                        shared_vars.times_embed_dhe_hash = []
                        shared_vars.times_embed_dhe_mlp = []


