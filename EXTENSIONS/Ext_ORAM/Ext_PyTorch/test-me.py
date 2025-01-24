import torch

import EmbeddingBagExt_ZT_PORAM
# import EmbeddingBagExt_ZT_CORAM

embSize = 16
rowCount = 1024

emb_lyr = EmbeddingBagExt_ZT_PORAM.EmbeddingBag(rowCount, embSize)
# emb_lyr = EmbeddingBagExt_ZT_CORAM.EmbeddingBag(rowCount, embSize)  



ind = torch.ones(5, dtype=torch.int64)
offsets = torch.arange(ind.shape[0], dtype=torch.int64) # no offsets in reality in LLM 
x = emb_lyr(ind, offsets)


print('\n')
print(x.shape)

