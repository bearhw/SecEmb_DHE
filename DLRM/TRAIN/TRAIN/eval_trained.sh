
conda activate pyth310

export OMP_NUM_THREADS=8
export MKL_NUM_THREADS=8

export CUDA_VISIBLE_DEVICES=0

##########
### KAGGLE
##########

rawPath=/localdata/umar/DATASETS/CRITEO/input_kaggle_processed/train.txt
processedPath=/localdata/umar/DATASETS/CRITEO/input_kaggle_processed/kaggleAdDisplayChallenge_processed.npz

# Table
python dlrm_s_pytorch-savebest.py  --arch-sparse-feature-size=16 --arch-mlp-bot="13-512-256-64-16" --arch-mlp-top="512-256-1" --data-generation=dataset --data-set=kaggle --raw-data-file=$rawPath --processed-data-file=$processedPath --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=128 --print-freq=1024 --print-time --test-freq=8192 --test-mini-batch-size=16384 --test-num-workers=8  --num-workers=8     --load-model=../TRAINED_MODELS/kaggle_table.pt     --embedding-representation 'table'    --dataset-multiprocessing     --use-gpu      --inference-only        2>&1 | tee   eval_kaggle_table.log


# DHE Uniform
DHEK=1024
python dlrm_s_pytorch-savebest.py --arch-sparse-feature-size=16 --arch-mlp-bot="13-512-256-64-16" --arch-mlp-top="512-256-1" --data-generation=dataset --data-set=kaggle  --raw-data-file=$rawPath --processed-data-file=$processedPath  --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=128 --print-freq=128 --print-time --test-freq=16384 --test-mini-batch-size=16384 --test-num-workers=8  --num-workers=8    --load-model=../TRAINED_MODELS/kaggle_dhe_uniform.pt   --embedding-representation 'dhe'     --dhe-k=$DHEK   --dhe-mlp-dims="$DHEK-512-256-16"     --use-gpu          --inference-only     2>&1 | tee eval_kaggle_dhe_uniform.log

# DHE Varied
DHEK=1024
python dlrm_s_pytorch-savebest.py --arch-sparse-feature-size=16 --arch-mlp-bot="13-512-256-64-16" --arch-mlp-top="512-256-1" --data-generation=dataset --data-set=kaggle  --raw-data-file=$rawPath --processed-data-file=$processedPath  --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=128 --print-freq=128 --print-time --test-freq=16384 --test-mini-batch-size=16384 --test-num-workers=8  --num-workers=8    --load-model=../TRAINED_MODELS/kaggle_dhe_varied.pt   --embedding-representation 'dhe'     --dhe-k=$DHEK   --dhe-mlp-dims="$DHEK-512-256-16"     --use-gpu          --inference-only     2>&1 | tee eval_kaggle_dhe_varied.log

# Cached DHE as TABLE (Uniform)
python dlrm_s_pytorch-savebest.py  --arch-sparse-feature-size=16 --arch-mlp-bot="13-512-256-64-16" --arch-mlp-top="512-256-1" --data-generation=dataset --data-set=kaggle --raw-data-file=$rawPath --processed-data-file=$processedPath --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=128 --print-freq=1024 --print-time --test-freq=8192 --test-mini-batch-size=16384 --test-num-workers=8  --num-workers=8     --load-model=../CONVERT_DHE2LS/STEP2/OUTPUT_MODELS/cached_kaggle.pt    --embedding-representation 'table'    --dataset-multiprocessing     --use-gpu      --inference-only        2>&1 | tee   eval_kaggle_table_converted.log


##########
#### TERABYTE
##########

rawPath=/localdata/umar/DATASETS/CRITEO/input_terabyte_10m_processed/day
processedPath=/localdata/umar/DATASETS/CRITEO/input_terabyte_10m_processed/terabyte_processed.npz

# Table
python dlrm_s_pytorch-savebest.py --arch-sparse-feature-size=64 --arch-mlp-bot="13-512-256-64" --arch-mlp-top="512-512-256-1" --max-ind-range=10000000 --data-generation=dataset --data-set=terabyte --raw-data-file=$rawPath --processed-data-file=$processedPath  --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=2048 --print-freq=1024 --print-time --test-mini-batch-size=16384 --test-num-workers=8  --test-freq=10240  --memory-map  --data-sub-sample-rate=0.875   --load-model=../TRAINED_MODELS/terabyte_table.pt   --embedding-representation 'table'             --inference-only             2>&1 | tee eval_terabyte_table.log
## --use-gpu 

# DHE Uniform
DHEK=1024
python dlrm_s_pytorch-savebest.py  --arch-sparse-feature-size=64 --arch-mlp-bot="13-512-256-64" --arch-mlp-top="512-512-256-1" --max-ind-range=10000000 --data-generation=dataset --data-set=terabyte --raw-data-file=$rawPath --processed-data-file=$processedPath  --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=2048 --print-freq=128 --print-time --test-mini-batch-size=16384 --test-num-workers=8  --test-freq=16384  --memory-map  --data-sub-sample-rate=0.875   --load-model=../TRAINED_MODELS/terabyte_dhe_uniform.pt     --embedding-representation 'dhe'     --dhe-k=$DHEK   --dhe-mlp-dims="$DHEK-512-256-64"  --use-gpu    --inference-only      2>&1 | tee eval_terabyte_dhe_uniform.log

# DHE Varied
DHEK=1024
python dlrm_s_pytorch-savebest.py  --arch-sparse-feature-size=64 --arch-mlp-bot="13-512-256-64" --arch-mlp-top="512-512-256-1" --max-ind-range=10000000 --data-generation=dataset --data-set=terabyte --raw-data-file=$rawPath --processed-data-file=$processedPath  --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=2048 --print-freq=128 --print-time --test-mini-batch-size=16384 --test-num-workers=8  --test-freq=16384  --memory-map  --data-sub-sample-rate=0.875   --load-model=../TRAINED_MODELS/terabyte_dhe_varied.pt     --embedding-representation 'dhe'     --dhe-k=$DHEK   --dhe-mlp-dims="$DHEK-512-256-64"  --use-gpu    --inference-only      2>&1 | tee eval_terabyte_dhe_varied.log

# Cached DHE as TABLE (Uniform)
python dlrm_s_pytorch-savebest.py --arch-sparse-feature-size=64 --arch-mlp-bot="13-512-256-64" --arch-mlp-top="512-512-256-1" --max-ind-range=10000000 --data-generation=dataset --data-set=terabyte --raw-data-file=$rawPath --processed-data-file=$processedPath  --loss-function=bce --round-targets=True --learning-rate=0.1 --mini-batch-size=2048 --print-freq=1024 --print-time --test-mini-batch-size=16384 --test-num-workers=8  --test-freq=10240  --memory-map  --data-sub-sample-rate=0.875   --load-model=../CONVERT_DHE2LS/STEP2/OUTPUT_MODELS/cached_terabyte.pt   --embedding-representation 'table'             --inference-only             2>&1 | tee eval_terabyte_table_converted.log


