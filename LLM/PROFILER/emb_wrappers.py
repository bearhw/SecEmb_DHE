import torch
import torch.nn as nn
import random


########## DHE

m = 1000000
p = 1000003

class DHE_Emb(nn.Module):
    def __init__(self, emb_dim, mlp_dims):
        super(DHE_Emb, self).__init__()

        # mlp_dims   # first is k
        mlp_dims += "-"+str(emb_dim)
        # print('dhe mlp_dims',mlp_dims)
        mlp_dims = [int(i) for i in mlp_dims.split('-')]

        k = mlp_dims[0]

        layers = nn.ModuleList()
        
        for i in range(0, len(mlp_dims) - 1):

            dim_in  = int(mlp_dims[i])
            dim_out = int(mlp_dims[i + 1])
            layer   = nn.Linear(dim_in, dim_out, bias = True)

            layers.append(layer)
            layers.append(torch.nn.ReLU())
            
        self.mlp_stacks = torch.nn.Sequential(*layers)
        # print('DHE Model Decoder',self.mlp_stacks)

        self.a = []
        self.b = []
        for i in range(0,k):
            self.a.append(random.randint(1,p-1))
            self.b.append(random.randint(0,p-1))
                    
        self.aa = torch.nn.Parameter(torch.LongTensor(self.a), requires_grad=False)
        self.bb = torch.nn.Parameter(torch.LongTensor(self.b), requires_grad=False)

    def forward(self, x):

        b = x.shape[0]
        t = x.shape[1]

        x = torch.flatten(x) 

        with torch.no_grad():
            x = torch.outer(x, self.aa).add(self.bb).fmod(p).fmod(m) # hashing, still integer

            x = (x / (m-1)) * 2 - 1 # uniform transformation, to float

        x = x.to(self.mlp_stacks[0].weight.dtype) 
        x = self.mlp_stacks(x)        
        x = torch.reshape(x, (b, t, -1))

        return x








########## Linear Scan

class LS_Emb(nn.Module):
    def __init__(self, vocab_size, embed_dim):
        super(LS_Emb, self).__init__()
        
        import EmbeddingBagExt_LinearScan
        self.wte = EmbeddingBagExt_LinearScan.EmbeddingBag(vocab_size, embed_dim)   


    def forward(self, x):

        b = x.shape[0]
        t = x.shape[1]

        x = torch.flatten(x) # b*t
        
        offsets = torch.arange(x.shape[0]) # no offsets really in LLM 
        x = self.wte(x,offsets)

        x = torch.reshape(x, (b, t, -1))

        return x





########## ZeroTrace PathORAM / CircuitORAM 

class ZT_ORAM_Emb(nn.Module):
    def __init__(self, vocab_size, embed_dim, oram_type):
        super(ZT_ORAM_Emb, self).__init__()

        if oram_type == "Path":
            import EmbeddingBagExt_ZT_PORAM
            self.wte = EmbeddingBagExt_ZT_PORAM.EmbeddingBag(vocab_size, embed_dim)      
        elif oram_type == "Circuit":
            import EmbeddingBagExt_ZT_CORAM
            self.wte = EmbeddingBagExt_ZT_CORAM.EmbeddingBag(vocab_size, embed_dim)      
        else:
            panic("Unknown ORAM type")


    def forward(self, x):

        b = x.shape[0]
        t = x.shape[1]

        x = torch.flatten(x) # b*t
        
        offsets = torch.arange(x.shape[0]) # no offsets really in LLM 
        x = self.wte(x,offsets)

        x = torch.reshape(x, (b, t, -1))

        return x