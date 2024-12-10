
conda activate pyth310



# CUSTOM_THREAD_COUNT=16  python   run_dhe_kaggle_e2e.py --inference --device 'cpu'      2>&1  | tee OUT.log  





CUSTOM_THREAD_COUNT=16  python   run_dhe_tera_e2e.py --inference --device 'cpu'      2>&1  | tee OUT.log  









