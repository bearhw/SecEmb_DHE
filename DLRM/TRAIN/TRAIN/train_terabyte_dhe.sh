
conda activate pyth310

export OMP_NUM_THREADS=16
export MKL_NUM_THREADS=16

export CUDA_VISIBLE_DEVICES=0

SEED=12345

rawPath=/localdata/umar/DATASETS/CRITEO/input_terabyte_10m_processed/day
processedPath=/localdata/umar/DATASETS/CRITEO/input_terabyte_10m_processed/terabyte_processed.npz


DHEK=1024

python dlrm_s_pytorch-savebest.py  --arch-sparse-feature-size=64 --arch-mlp-bot="13-512-256-64" --arch-mlp-top="512-512-256-1" --max-ind-range=10000000 --data-generation=dataset --data-set=terabyte --raw-data-file=$rawPath --processed-data-file=$processedPath  --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=2048 --print-freq=128 --print-time --test-mini-batch-size=16384 --test-num-workers=8  --test-freq=16384  --memory-map  --data-sub-sample-rate=0.875  --save-model=terabyte_dhe_k$DHEK\_seed$SEED.pt    --embedding-representation 'dhe'     --dhe-k=$DHEK   --dhe-mlp-dims="$DHEK-512-256-64"    --numpy-rand-seed=$SEED       2>&1 | tee train_terabyte_dhe_k$DHEK\_seed$SEED.log

# --use-gpu  
