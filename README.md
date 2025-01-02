
## README

Public Artifact for the HPCA'25 paper "Efficient Memory Side-Channel Protection for Embedding Generation in Machine Learning".

Zenodo repository link (for pretrained models): https://zenodo.org/records/14578272

---


### SETUP INSTRUCTIONS

This requires a CPU with a Trusted Execution Environment (TEE); we specifically target Intel CPUs with SGX. 
Hence, we require at least a 3rd Gen (Ice Lake or later) Intel Scalable Xeon Processor with SGX enabled and >64GB secure memory.
We also required AVX-512 support.
SGX further needs Gramine library OS installed (https://gramine.readthedocs.io/en/latest/installation.html) and a signing key generated.

For Python, we use an Anaconda (www.anaconda.com) based flow.
First create a new environment `conda create -n pyth310 python=3.10`.

Then install the following packages:
`pip install torch torchvision torchrec future numpy tqdm onnx pydot scikit-learn tensorboard shapely scipy matplotlib transformers datasets tiktoken wandb tqdm`.

Also install the assembler `conda install anaconda::nasm` and `libssl-dev` or equivalent.

---

### RUNNING INSTRUCTIONS

Before running experiments, install our custom-implemented PyTorch extensions in the `EXTENSIONS` folder.
First, inside the dir `EXTENSIONS/Ext_ORAM/ZT_Clean`, run `make` and build our modified ZeroTrace.
Then inside each sub-folder `EXTENSIONS/Ext_*`, run `source cmd-setup.sh` to install the extensions (into the conda environment).
Note: where the `ZT_Clean` ZeroTrace is built, the absolute fixed paths to ZT_Clean need to be edit in `EXTENSIONS/Ext_ORAM/Ext_PyTorch/setup.py`, and also in all instances of `pytorch.manifest.template` in the repository.

---

#### DLRM

##### TRAINING 

The `DLRM/TRAIN/DATASETS` directory has instructions for training-dataset download and preparation (i.e. Criteo).

Inside the `DLRM/TRAIN/TRAIN` directory, run the various `train_*.sh` scripts (via `source <script_name>.sh`) for training Table, DHE-Uniform and DHE-Varied models for both Kaggle and Terabyte datasets (for training either DHE-Uniform or Varied, one first needs to respectively copy and rename either `dhebag--UNIFORM.py` or `dhebag--VARIED.py` to `dhebag.py`).
The training scripts will generate `*.log` and save model checkpoints `*.pt`.

The `DLRM/TRAIN/TRAIN` directory also includes a script `eval_trained.sh` that can be run for evaluating and printing existing pre-trained model accuracy for Table/DHE-Uniform/DHE-Varied (again rename appropriate `dhebag.py`). The pretrained models are to be in `DLRM/TRAIN/TRAINED_MODELS` and can be downloaded from our Zenodo repository under `DLRM_PRETRAINED_MODELS.zip`.

The `DLRM/TRAIN/CONVERT_DHE2LS` directory has scripts for caching DHE as a table representation. We provide scripts for DHE-Uniform only (hybrid model conversion can be performed similarly).
Inside `CONVERT_DHE2LS/STEP1`, first run `source run.sh` to sweep DHE model output using input IDs, and then run inside `CONVERT_DHE2LS/STEP2`, run `source run.sh` to output a `pt` torch model that has table representation but using DHE weights. Sample converted models can be downloaded from our Zenodo repository under `DLRM_CONVERTED_MODELS.zip`.
Moreover, the `DLRM/TRAIN/TRAIN/eval_trained.sh` inference script also has commands (under `Cached DHE as TABLE `) to show that when DHE is converted to a table representation, the accuracy is the same.


##### PROFILING

The first step is to profile the latencies for various embedding generation techniques via running `source SCRIPTS/run_profiling.sh` inside the directory `DLRM/PROFILER_INFERENCE`. 
This will output `.log` files in `DLRM/PROFILER_INFERENCE/RESULTS_OUTPUT` for various embedding generation models and table sizes.

Note: The scripts can be run either inside or outside SGX by choosing one of the `cmd=` in the script file `DLRM/PROFILER_INFERENCE/SCRIPTS/run_profiling.sh`. 
For running inside SGX, one must first do `source sgx_maker.sh` inside `DLRM/PROFILER_INFERENCE` to generate the enclave setup files.
Running inside SGX will add a very small overhead.

Then, inside `DLRM/PROFILER_INFERENCE/RESULTS_OUTPUT`, run `python DLRM/PROCESS_RESULTS/EXTRACT_CSV/extract-to-csv.py` to extract the relevant latencies from the logs into CSV files (under the `TimeEmb` column).

The CSV files have latencies which are visualized as plots using the jupyter notebook `PROCESS_RESULTS/CALC_THRESHOLDS/ProfilingResultsThresholds.ipynb` (adjust the path to `RESULTS_OUTPUT/ParsedCSVs` if needed).
The notebook also calculates the thresholds at which DHE is better than Linear Scan, for various execution configurations (batch/thread counts), and outputs the thresholds as CSV files.
The thresholds are used in the Hybrid model during end-to-end inference (tables below the thresholds are supposed to be allocated to Linear Scan in the Hybrid model, and above to DHE).


#### INFERENCE

For the end-to-end inference for various embedding generation techniques, do `source SCRIPTS/run_e2e.sh` inside the directory `DLRM/PROFILER_INFERENCE`. 
This will again output logs in `DLRM/PROFILER_INFERENCE/RESULTS_OUTPUT` which can be converted to CSV files containing the latencies (under the `TimeTotal` column).
Also, inference can be run either inside/outside SGX by editing the `cmd=` inside the script.


---



#### LLM

Activate the conda environment `pyth310` for all experiments.

##### TRAINING 

We provide finetuning scripts for GPT-2 medium LLM, for the original table vocabulary table and its replacement DHE embedding layer.

Prepare the OpenWebText dataset using the python script in the `LLM/TRAIN/data` folder.

Launch Table representation or DHE finetuning in the respective `LLM/TRAIN/nanoGPT_*` folders using the `launch_finetune.sh` script. 

Existing model accuracies can be evaluated using the `launch_eval.sh` script in the same folder. 
Pretrained Table and DHE models can be downloaded from our Zenodo repository under `DLRM_CONVERTED_MODELS.zip`. The `pt` model files are to be in the proper `LLM/TRAIN/nanoGPT_***/output-model-owt` respectively.
The reported train and validation losses can be converted to perplexity values using `exp(loss)`.

The trained models can be prompted for generation using the `launch_sample.sh` script in the same folder.


##### PROFILING / INFERENCE 

To run profiling experiments on various embeddding generation techniques and GPT-2 medium sized token embedding table, run `run_profiler.sh` inside the `LLM/PROFILER` dir.
For running inside SGX, first run `sgx_maker.sh` in the same dir and adjust the `cmd=` in the script.
The results in the `LLM/PROFILER/RESULTS_OUTPUT` dir can be plot with the notebook in the , and plot the `LLM/PROFILER/PROCESS_RESULTS` dir.

To run end-to-end inference profiling on GPT-2 medium with various embedding generation techniques, first install the patched HuggingFace `transformers` lib by running `install.sh` in `LLM/INFERENCE/HF_LIB`.
Then run `setup_model.sh` inside `LLM/INFERENCE/INFER` to download the model config.  
Then run `run_inf.sh` inside `LLM/INFERENCE/INFER` to launch the inference.  
For running inside SGX, first run `sgx_maker.sh` in the same dir and adjust the `cmd=` in the script.
Again observe results in `LLM/INFERENCE/INFER/RESULTS_OUTPUT` dir and summarize them with the notebook in the `LLM/INFERENCE/PROCESS_RESULTS` dir.



---


#### COMMON

Run the python scripts in the `MISC` folder to output model memory sizes for various embedding generation techniques, for both DLRM and LLM.




