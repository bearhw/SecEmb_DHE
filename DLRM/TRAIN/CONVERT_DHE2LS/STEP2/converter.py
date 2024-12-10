import torch

# terabyte or kaggle
kaggle=1


####  load dhe params
if kaggle:
    x = torch.load('kaggle_dhe_uniform.pt',map_location=torch.device('cpu'))
else:
    x = torch.load('terabyte_dhe_uniform.pt',map_location=torch.device('cpu'))


#### any sample table file for copying into
if kaggle:
    y = torch.load('kaggle_table.pt',map_location=torch.device('cpu'))
else:
    y = torch.load('terabyte_table.pt',map_location=torch.device('cpu'))


# for key in x['state_dict']:
#     print(key, x['state_dict'][key].shape)
# print('\n')
# for key in y['state_dict']:
#     print(key, y['state_dict'][key].shape)
# print('\n')

# copying top and bottom MLP
for key in x['state_dict']:    
    if key in y['state_dict']:        
        print('common key',key)
        y['state_dict'][key].copy_( x['state_dict'][key]  )
print('\n')



# copying cached DHE output

if True:
  for i in range(26):
    
    if kaggle:
        path = f'../STEP1/CACHED_TABLES/kaggle_tbl{i}.pt'
    else:        
        path = f'../STEP1/CACHED_TABLES/tera_tbl{i}.pt'

    cached = torch.load(path, map_location=torch.device('cpu'))

    
    key = f'emb_l.{i}.weight'

    print(key, cached.shape, y['state_dict'][key].shape)
    
    y['state_dict'][key].copy_( cached )


print('saving')
torch.save(y, "OUTPUT_MODELS/cached_kaggle.pt")
# torch.save(y, "OUTPUT_MODELS/cached_terabyte.pt")

# 

