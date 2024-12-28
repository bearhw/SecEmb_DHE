
### TRAINING 

The `DATASETS` directory has instructions for training-dataset preparation (i.e. Criteo).

`TRAIN` directory has `train_*` scripts for training table, DHE uniform and DHE varied models (need to rename desired the desired type of DHE to `dhebag.py`)
Also includes a script `eval_trained` for evaluating the trained model accuracy.

The `CONVERT_DHE2LS` directory has scripts for caching DHE as a table representation. We provide scripts for uniform only (hybrid model conversion can be performed similarly). The above accuracy inference script also shows the same accuracy when DHE is converted to a table representation.


### PROFILER / INFERENCE 

The first step is to profile the latencies for various embedding generation techniques via `source SCRIPTS/run_profiling.sh`.

Then the relevant latencies are extracted from the logs into CSV files via `PROCESS_RESULTS/EXTRACT_CSV/extract-to-csv.py`.

The thresholds for various execution configuration are calculated via the notebook `PROCESS_RESULTS/CALC_THRESHOLDS/ProfilingResultsThresholds.ipynb`.

End-to-end inference for various embedding generation techniques can be launched via `source SCRIPTS/run_e2e.sh`, and the latencies can be read in the log/csv files. 
