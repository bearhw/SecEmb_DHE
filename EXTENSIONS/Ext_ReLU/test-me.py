import os

thread_count = "1"
print("thread_count", thread_count)
os.environ["OMP_NUM_THREADS"] = thread_count
os.environ["MKL_NUM_THREADS"] = thread_count
os.environ["OPENBLAS_NUM_THREADS"] = thread_count
os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
os.environ["NUMEXPR_NUM_THREADS"] = thread_count


import torch

torch.set_printoptions(precision=4, sci_mode=False)
 

import ReLU_Ext
class EXT_RELU_MOD(torch.nn.Module):
    def __init__(self):
        super(EXT_RELU_MOD, self).__init__()
        pass    
    def forward(self, x):
        return ReLU_Ext.forward(x)
    def string(self):
        pass


modrelu = EXT_RELU_MOD()

size = 1024
B = 64
w = torch.randn((B, size))
print('orig', w)

y = modrelu(w)
print("ext output  ", y, y.shape, y.dtype)
    
z = torch.nn.functional.relu(w) 
    
diff = torch.sum(torch.abs(y - z))
print('diff total',diff)    

