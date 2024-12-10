import os

thread_count = "1"
print("thread_count", thread_count)
os.environ["OMP_NUM_THREADS"] = thread_count
os.environ["MKL_NUM_THREADS"] = thread_count
os.environ["OPENBLAS_NUM_THREADS"] = thread_count
os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
os.environ["NUMEXPR_NUM_THREADS"] = thread_count


import time

import torch

torch.set_printoptions(precision=4, sci_mode=False)


import ReLU_Ext

size = 16
B = 4
w = torch.randn((B, size))
print('orig', w)

y = ReLU_Ext.forward(w)
print("output  ", y, y.shape, y.dtype)
    
