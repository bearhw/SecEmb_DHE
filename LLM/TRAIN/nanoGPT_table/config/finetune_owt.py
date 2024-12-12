import time
import os

out_dir = 'output-model-owt'
eval_interval = 50
eval_iters = 100

wandb_log = True # feel free to turn on
wandb_project = 'owt_ft_gpt_medium_table'
wandb_run_name = 'ft-' + str(time.time())

dataset = 'openwebtext'
init_from = 'gpt2-medium' # this is the largest GPT-2 model
ckpt_path=out_dir+'/ckpt.pt'
if os.path.exists(ckpt_path):
    init_from = 'resume' # ckpt

# only save checkpoints if the validation loss improves
always_save_checkpoint = False

# the number of examples per iter:
# 1 batch_size * 32 grad_accum * 1024 tokens = 32,768 tokens/iter
# shakespeare has 301,966 tokens, so 1 epoch ~= 9.2 iters
batch_size = 8
gradient_accumulation_steps = 40
max_iters = 75000

# finetune at constant LR
learning_rate = 5e-5
decay_lr = False
