
conda activate pyth310

export OMP_NUM_THREADS=16
export MKL_NUM_THREADS=16

export CUDA_VISIBLE_DEVICES=0

SEED=12345

rawPath=/localdata/umar/DATASETS/CRITEO/input_kaggle_processed/train.txt
processedPath=/localdata/umar/DATASETS/CRITEO/input_kaggle_processed/kaggleAdDisplayChallenge_processed.npz
 

python dlrm_s_pytorch-savebest.py  --arch-sparse-feature-size=16 --arch-mlp-bot="13-512-256-64-16" --arch-mlp-top="512-256-1" --data-generation=dataset --data-set=kaggle --raw-data-file=$rawPath --processed-data-file=$processedPath --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=128 --print-freq=1024 --print-time --test-freq=8192 --test-mini-batch-size=16384 --test-num-workers=8  --num-workers=8     --save-model=kaggle_table_seed$SEED.pt    --embedding-representation 'table'    --dataset-multiprocessing          --numpy-rand-seed=$SEED            2>&1 | tee train_kaggle_table_seed$SEED.log


 


