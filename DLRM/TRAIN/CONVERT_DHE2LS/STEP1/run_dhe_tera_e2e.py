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
    
    trained = torch.load('../../TRAINED_MODELS/terabyte_dhe_uniform.pt',map_location=torch.device('cpu'))
    
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
        
    # process = psutil.Process()
    
    
    # list_of_output_lists.append(out)
        
    # exit()
    
    tableSizes_list = '9980333-36084-17217-7378-20134-3-7112-1442-61-9758201-1333352-313829-10-2208-11156-122-4-970-14-9994222-7267859-9946608-415421-12420-101-36'
    table_ctr = 0
    
    for tableSize in tableSizes_list.split('-'):
    #     print(tableSize)
    # for tableSize in ['1101','2343','100000']:
    # for tableSize in ['128','133']:
        
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
        torch.save(tnsr, f"CACHED_TABLES/tera_tbl{table_ctr}.pt")
        table_ctr += 1
        
        
        # print('debug final-0',out[0].shape)
            
    exit()

    
    
    
    
    
    
    
    
    
    
    
    
    

    # # Training
    # if args.train == True:
    #     if args.device == 'cpu' or args.device == 'gpu':
    #         model.train()
    #         timing_vector     = []
    #         timing_vector_fwd = []
    #         timing_vector_bwd = []
    #         for epoch in range(args.num_epochs):
    #             print('Epoch {} ===================='.format(epoch))
    #             for i, inputBatch in enumerate(dataloader):
    #                 x_dense, x_offsets, x_indices, y = inputBatch
    #                 t0      = time()
    #                 outputs = model(x_dense.to(device), x_offsets.to(device), x_indices.to(device))
    #                 t1      = time()
    #                 loss    = model.loss_fn(outputs, y.to(device))
    #                 optimizer.zero_grad()
    #                 loss.backward()
    #                 optimizer.step()
    #                 t2 = time()
    #                 timing_vector.append(1000*(t2 - t0))
    #                 timing_vector_fwd.append(1000*(t1 - t0))
    #                 timing_vector_bwd.append(1000*(t2 - t1))
    #                 if (i%args.print_freq == 0):
    #                     print('Epoch {} Batch {}: {:.3f} ({:.3f}, {:.3f}) ms'.format(
    #                         epoch, i, timing_vector[-1], timing_vector_fwd[-1], timing_vector_bwd[-1]))
    #         print('\nTotal Training Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector)*len(timing_vector)))
    #         print('Total Training Time: {:.3f} ms/it'.format(np.sum(timing_vector)))
    #         print('Median Training Time: {:.3f} ms/it'.format(np.median(timing_vector)))
    #         print('Mean Training Time: {:.3f} ms/it\n'.format(np.mean(timing_vector)))

    #         print('\nTotal FWD Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector_fwd)*len(timing_vector_fwd)))
    #         print('Total FWD Time: {:.3f} ms/it'.format(np.sum(timing_vector_fwd)))
    #         print('Median FWD Time: {:.3f} ms/it'.format(np.median(timing_vector_fwd)))
    #         print('Mean FWD Time: {:.3f} ms/it\n'.format(np.mean(timing_vector_fwd)))

    #         print('\nTotal BWD Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector_bwd)*len(timing_vector_bwd)))
    #         print('Total BWD Time: {:.3f} ms/it'.format(np.sum(timing_vector_bwd)))
    #         print('Median BWD Time: {:.3f} ms/it'.format(np.median(timing_vector_bwd)))
    #         print('Mean BWD Time: {:.3f} ms/it\n'.format(np.mean(timing_vector_bwd)))


    # # Inference
    # if args.inference == True:
    #     if args.device == 'cpu' or args.device == 'gpu':
    #         model.eval()
    #         timing_vector = []
    #         for epoch in range(args.num_epochs):
    #             print('Epoch {} ===================='.format(epoch))
    #             for i, inputBatch in enumerate(dataloader):
    #                 x_dense, x_offsets, x_indices, _ = inputBatch
    #                 t0      = time()
    #                 outputs = model(x_dense.to(device), x_offsets.to(device), x_indices.to(device))
    #                 t1      = time()
    #                 timing_vector.append(1000*(t1 - t0))
    #                 if (i%args.print_freq == 0):
    #                     print('Epoch {} Batch {}: {:.3f} ms'.format(epoch, i, timing_vector[-1]))
                        
    #         # print(timing_vector)
    #         timing_vector.pop(0)
            
    #         print('\nTotal Inference Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector)*len(timing_vector)))
    #         print('Total Inference Time: {:.3f} ms/it'.format(np.sum(timing_vector)))
    #         print('Median Inference Time: {:.3f} ms/it'.format(np.median(timing_vector)))
    #         print('Mean Inference Time: {:.3f} ms/it\n'.format(np.mean(timing_vector)))

    #         shared_vars.times_bot_mlp.pop(0)
    #         shared_vars.times_embed.pop(0)
    #         shared_vars.times_interact.pop(0)
    #         shared_vars.times_top_mlp.pop(0)
    #         print('Mean Inference Time for Bottom MLP:  {:.3f} ms/it'.format(np.mean(shared_vars.times_bot_mlp)))
    #         print('Mean Inference Time for Embedding:   {:.3f} ms/it'.format(np.mean(shared_vars.times_embed)))
    #         print('Mean Inference Time for Interaction: {:.3f} ms/it'.format(np.mean(shared_vars.times_interact)))
    #         print('Mean Inference Time for Top MLP:     {:.3f} ms/it'.format(np.mean(shared_vars.times_top_mlp)))
    #         print('Total:                               {:.3f} ms/it'.format(np.mean(shared_vars.times_bot_mlp)+np.mean(shared_vars.times_embed)+np.mean(shared_vars.times_interact)+np.mean(shared_vars.times_top_mlp)))
    #         print('')

    #         print('Mean Inference Time for Embedding Hash:   {:.3f} ms/it'.format(np.mean(shared_vars.times_embed_dhe_hash)))
    #         print('Mean Inference Time for Embedding MLP:    {:.3f} ms/it'.format(np.mean(shared_vars.times_embed_dhe_mlp)))
    #         print('Total:                                    {:.3f} ms/it'.format(np.mean(shared_vars.times_embed_dhe_hash)+np.mean(shared_vars.times_embed_dhe_mlp)))
    #         print('')
            
    #         print('Result:')
    #         res_str = ' {:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f} \n'.format(np.mean(timing_vector), np.mean(shared_vars.times_bot_mlp), np.mean(shared_vars.times_embed), np.mean(shared_vars.times_interact), np.mean(shared_vars.times_top_mlp),np.mean(shared_vars.times_embed_dhe_hash),np.mean(shared_vars.times_embed_dhe_mlp))
    #         res_str = str(torch.get_num_threads()) + ',' + str(args.batch_size) +  ',' + str(args.dense_dim) + ',' + str(len(args.table_sizes)) + ',' + str(args.emb_dim) + ',' + str(args.num_lookups) + ',' + str(args.dhe_k) + ',' + '     ' + res_str
    #         result_str += res_str 
    #         print('')

    


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
    shared_vars.times_bot_mlp = []
    shared_vars.times_embed = []
    shared_vars.times_interact = []
    shared_vars.times_top_mlp = []
    shared_vars.times_embed_dhe_hash = []
    shared_vars.times_embed_dhe_mlp = []

    ###################
    ## sweep parameters

    # batchSize_list =  [1, *range(32,128+1,32)]
    # batchSize_list = [ 1, 8, 16, 32, 64, 128 ]
    # # batchSize_list = [ 32 ]
    batchSize_list = [ 1 ]
    
    denseFeatures = 13
    args.dense_dim = denseFeatures

    # adjust table size here as well if needed
    tableSizes_list=['100000'] # if 1 table size only
    if 1: 
        # add more tables
        # for i in range(5,30+1,5):
        # for i in range(5,15+1,5):
        # for i in range(5,10+1,5):
        # for i in [10]:
        for i in [3]:
        # for i in [5]:
            tbl_str = ""
            for y in range(1,i+1):
                    tbl_str += str(100000) + '-' 
            tableSizes_list.append ( tbl_str[:-1] )
    print('tableSizes_list ', tableSizes_list)        
    print('')

    # embSize_list = range(16,64+1,16)
    # embSize_list = [ 16, 64 ]
    embSize_list = [ 64 ]
    # embSize_list = [ 16 ]

    # dheK_list = range(256,3072+1,256)
    # dheK_list = [ 1024, 2048 ]
    # dheK_list = [ 1024, 2048, 3072, 4096, 5120, 6144 ]
    # dheK_list = [ 6144 ]
    dheK_list = [ 1024 ]

    # numLookups_list =  [1,3,5]
    # numLookups_list =  [1,5,10]
    numLookups_list = [ 1 ]
    # numLookups_list = [ 1,5 ]


    mlp_bot_dims_str =  "13-512-256-64-"
    mlp_top_dims_str =  "512-256-1"



    # # criteo 
    # batchSize_list = [ 1, 16, 32, 64 ]
    # args.dense_dim = 13
    # numLookups_list = [ 1 ]
    # dheK_list = [ 1024 ]

    # # kaggle
    # embSize_list = [ 16 ]
    # tableSizes_list = ['3-4-10-15-18-24-27-105-305-583-633-1460-2173-3194-5652-5683-12517-14992-93145-142572-286181-2202608-5461306-7046547-8351593-10131227']
    # mlp_bot_dims_str =  "13-512-256-64-" #-16 
    # mlp_top_dims_str =  "512-256-1"

    # # terabyte
    embSize_list = [ 64 ]
    tableSizes_list = ['9980333-36084-17217-7378-20134-3-7112-1442-61-9758201-1333352-313829-10-2208-11156-122-4-970-14-9994222-7267859-9946608-415421-12420-101-36']
    mlp_bot_dims_str =  "13-512-256-" #-64
    mlp_top_dims_str =  "512-512-256-1"



    # randomize
    if 0:
        x = [int(i) for i in tableSizes_list[0].split('-')]
        import random
        random.seed()
        random.shuffle(x)
        x = [str(i) for i in x]
        tableSizes_list = ['-'.join(x)]










    # for threads vary via env var CUSTOM_THREAD_COUNT




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
                        # args.dhe_mlp_dims = str(dheK)+"-128-"+str(embSize)
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


    print('')
    print('FINAL')
    print(result_str)
 


    print('')    
    print('num_expr ', num_expr)    
    print('')


    print('tableSizes_list ', tableSizes_list)        
    print('')

    print("the number of cpu threads: {} ".format(torch.get_num_threads()))
    print('')
