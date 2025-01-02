import os

thread_count = "16"
print('thread_count', thread_count)
os.environ["OMP_NUM_THREADS"] = thread_count
os.environ["MKL_NUM_THREADS"] = thread_count
os.environ["OPENBLAS_NUM_THREADS"] = thread_count
os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
os.environ["NUMEXPR_NUM_THREADS"] = thread_count


import time

import torch

torch.set_printoptions(precision=4, sci_mode=False)

 
import Argmax_Ext




def test_new():
        
    B = 3
    size = 50257
    w = torch.randn((B, size))      
    print('input   ',w.shape, w.dtype)
    
    y = Argmax_Ext.forward(w)
    print('ext out       ',y, y.shape, y.dtype)

    z = torch.argmax(w, dim=-1)
    print('baseline out  ', z, z.shape, z.dtype)


test_new()
