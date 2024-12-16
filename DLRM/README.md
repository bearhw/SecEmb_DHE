
### TRAINING 

The `DATASETS` directory has instructions for training-dataset preparation.

`TRAIN` directory has `train_*` scripts for training table, DHE uniform and DHE varied models (need to rename desired the desired type of DHE to `dhebag.py`)
Also includes a script `eval_trained` for evaluating trained model accuracy.

The `CONVERT_DHE2LS` directory has scripts for caching DHE as a table representation. We provide scripts for uniform only (hybrid model conversion can be performed similarly).

### PROFILER / INFERENCE 

Has scripts for profiling, threshold calculation and end-to-end inference.

`TO-DO`