
conda activate pyth310

python sample.py \
    --init_from=resume \
    --start="What is the answer to life, the universe, and everything?" \
    --num_samples=3 --max_new_tokens=100
