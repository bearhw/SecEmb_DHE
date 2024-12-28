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
    args_dhe['table_size_threshold']   = args.table_size_threshold
    args_dhe['dhe_varying'] = args.dhe_varying
    
    print('---------- DHE Parameters: ----------')
    for key, val in args_dhe.items():
        print('{} : {}'.format(key, val))
    print('--------------------')


    # Default: CPU, GPU
    if 1:
        from dhe_dlrm_hybrid import DLRM_DHE
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
        model = DLRM_DHE(
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

    # Training
    if args.train == True:
        if args.device == 'cpu' or args.device == 'gpu':
            model.train()
            timing_vector     = []
            timing_vector_fwd = []
            timing_vector_bwd = []
            for epoch in range(args.num_epochs):
                print('Epoch {} ===================='.format(epoch))
                for i, inputBatch in enumerate(dataloader):
                    x_dense, x_offsets, x_indices, y = inputBatch
                    t0      = time()
                    outputs = model(x_dense.to(device), x_offsets.to(device), x_indices.to(device))
                    t1      = time()
                    loss    = model.loss_fn(outputs, y.to(device))
                    optimizer.zero_grad()
                    loss.backward()
                    optimizer.step()
                    t2 = time()
                    timing_vector.append(1000*(t2 - t0))
                    timing_vector_fwd.append(1000*(t1 - t0))
                    timing_vector_bwd.append(1000*(t2 - t1))
                    if (i%args.print_freq == 0):
                        print('Epoch {} Batch {}: {:.3f} ({:.3f}, {:.3f}) ms'.format(
                            epoch, i, timing_vector[-1], timing_vector_fwd[-1], timing_vector_bwd[-1]))
            print('\nTotal Training Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector)*len(timing_vector)))
            print('Total Training Time: {:.3f} ms/it'.format(np.sum(timing_vector)))
            print('Median Training Time: {:.3f} ms/it'.format(np.median(timing_vector)))
            print('Mean Training Time: {:.3f} ms/it\n'.format(np.mean(timing_vector)))

            print('\nTotal FWD Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector_fwd)*len(timing_vector_fwd)))
            print('Total FWD Time: {:.3f} ms/it'.format(np.sum(timing_vector_fwd)))
            print('Median FWD Time: {:.3f} ms/it'.format(np.median(timing_vector_fwd)))
            print('Mean FWD Time: {:.3f} ms/it\n'.format(np.mean(timing_vector_fwd)))

            print('\nTotal BWD Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector_bwd)*len(timing_vector_bwd)))
            print('Total BWD Time: {:.3f} ms/it'.format(np.sum(timing_vector_bwd)))
            print('Median BWD Time: {:.3f} ms/it'.format(np.median(timing_vector_bwd)))
            print('Mean BWD Time: {:.3f} ms/it\n'.format(np.mean(timing_vector_bwd)))


    # Inference
    if args.inference == True:
        if args.device == 'cpu' or args.device == 'gpu':
            model.eval()
            timing_vector = []
            for epoch in range(args.num_epochs):
                print('Epoch {} ===================='.format(epoch))
                for i, inputBatch in enumerate(dataloader):
                    x_dense, x_offsets, x_indices, _ = inputBatch
                    t0      = time()
                    outputs = model(x_dense.to(device), x_offsets.to(device), x_indices.to(device))
                    t1      = time()
                    timing_vector.append(1000*(t1 - t0))
                    if (i%args.print_freq == 0):
                        print('Epoch {} Batch {}: {:.3f} ms'.format(epoch, i, timing_vector[-1]))
                        
            # print(timing_vector)
            timing_vector.pop(0)
            
            print('\nTotal Inference Time (Stable): {:.3f} ms/it'.format(np.median(timing_vector)*len(timing_vector)))
            print('Total Inference Time: {:.3f} ms/it'.format(np.sum(timing_vector)))
            print('Median Inference Time: {:.3f} ms/it'.format(np.median(timing_vector)))
            print('Mean Inference Time: {:.3f} ms/it\n'.format(np.mean(timing_vector)))

            shared_vars.times_bot_mlp.pop(0)
            shared_vars.times_embed.pop(0)
            shared_vars.times_interact.pop(0)
            shared_vars.times_top_mlp.pop(0)
            print('Mean Inference Time for Bottom MLP:  {:.3f} ms/it'.format(np.mean(shared_vars.times_bot_mlp)))
            print('Mean Inference Time for Embedding:   {:.3f} ms/it'.format(np.mean(shared_vars.times_embed)))
            print('Mean Inference Time for Interaction: {:.3f} ms/it'.format(np.mean(shared_vars.times_interact)))
            print('Mean Inference Time for Top MLP:     {:.3f} ms/it'.format(np.mean(shared_vars.times_top_mlp)))
            print('Total:                               {:.3f} ms/it'.format(np.mean(shared_vars.times_bot_mlp)+np.mean(shared_vars.times_embed)+np.mean(shared_vars.times_interact)+np.mean(shared_vars.times_top_mlp)))
            print('')

            print('Mean Inference Time for Embedding Hash:   {:.3f} ms/it'.format(np.mean(shared_vars.times_embed_dhe_hash)))
            print('Mean Inference Time for Embedding MLP:    {:.3f} ms/it'.format(np.mean(shared_vars.times_embed_dhe_mlp)))
            print('Total:                                    {:.3f} ms/it'.format(np.mean(shared_vars.times_embed_dhe_hash)+np.mean(shared_vars.times_embed_dhe_mlp)))
            print('')
            
            print('Result:')
            res_str = ' {:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f}, {:.4f} \n'.format(np.mean(timing_vector), np.mean(shared_vars.times_bot_mlp), np.mean(shared_vars.times_embed), np.mean(shared_vars.times_interact), np.mean(shared_vars.times_top_mlp),np.mean(shared_vars.times_embed_dhe_hash),np.mean(shared_vars.times_embed_dhe_mlp))
            res_str = str(torch.get_num_threads()) + ',' + str(args.batch_size) +  ',' + str(args.dense_dim) + ',' + str(len(args.table_sizes)) + ',' + str(args.emb_dim) + ',' + str(args.num_lookups) + ',' + str(args.dhe_k) + ',' + '     ' + res_str
            result_str += res_str 
            print('')

    


if __name__ == '__main__':
    

    # Experiment Arguments
    args = parse_configurations()
    print('DEFAULT args ', args)

    # once only
    args.precision    = eval(args.precision)

    result_str = ""

    # warm-up run
    main(args)    
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

    # batchSize_list = [ 1, 8, 16, 32, 64, 128 ]
    batchSize_list = [ 32 ]
    
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
    tableSizes_list = ['3-4-10-15-18-24-27-105-305-583-633-1460-2173-3194-5652-5683-12517-14992-93145-142572-286181-2202608-5461306-7046547-8351593-10131227']
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
