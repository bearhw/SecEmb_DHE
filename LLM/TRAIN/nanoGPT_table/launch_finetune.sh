
conda activate pyth310

python train.py config/finetune_owt.py 2>&1 | tee -a OUT_TRAIN.log
