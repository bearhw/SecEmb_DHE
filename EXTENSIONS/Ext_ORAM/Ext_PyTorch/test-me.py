import torch

print('====================',  ' test')

 

import EmbeddingBagExt_LinearScan
import EmbeddingBagExt_ZT_PORAM
import EmbeddingBagExt_ZT_CORAM

emb_lyr = EmbeddingBagExt_ZT_PORAM.EmbeddingBag(1024, 16)
# emb_lyr = EmbeddingBagExt_ZT_CORAM.EmbeddingBag(1024, 16)       # , "sum")




# emb_lyr = EmbeddingBagExt_LinearScan.EmbeddingBag(1024, 24)




wte = emb_lyr

x = torch.ones(4, 16, dtype=torch.int64)

b = x.shape[0]
t = x.shape[1]

x = torch.flatten(x) # bt
print('==================== x ', x.shape)

offsets = torch.arange(x.shape[0], dtype=torch.int64) # no offsets in reality in LLM 
x = wte(x,offsets)

 

x = torch.reshape(x, (b, t, -1))

output = x 

print('==================== o', output.shape)
# print(output)



