import os

thread_count = "1"
print('thread_count', thread_count)
os.environ["OMP_NUM_THREADS"] = thread_count
os.environ["MKL_NUM_THREADS"] = thread_count
os.environ["OPENBLAS_NUM_THREADS"] = thread_count
os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
os.environ["NUMEXPR_NUM_THREADS"] = thread_count


import torch
torch.set_printoptions(precision=4,sci_mode=False)


from EmbeddingBagExt_LinearScan import EmbeddingBag


def test_simple():
        
    embsz = 64
    emb_1 = EmbeddingBag(512, embsz, "sum")
    
    w = torch.randn((512, embsz))  

    indices = torch.tensor([313, 1, 511], dtype=torch.int64)
    offsets = torch.tensor([0, 1, 2], dtype=torch.int64)

    print(w.dtype)
    emb_1.setWeights(w)

    y = emb_1(indices, offsets)
    
    print('ext returned ', y)

    print('\n\n\n')
    print('orig w       ', w[313,:])
    print('orig w       ', w[1,:])
    print('orig w       ', w[511,:])

test_simple()

