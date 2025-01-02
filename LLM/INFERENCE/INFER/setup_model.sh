
conda activate pyth310  

# download gpt2 from huggingface 
python -c 'from huggingface_hub import snapshot_download; snapshot_download(repo_id="mohdumar/gpt2-medium-untied", local_dir="mohdumar-gpt2-medium-untied", local_dir_use_symlinks=False)'
