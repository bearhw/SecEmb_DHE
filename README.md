
### README

Public Artifact for the HPCA'25 paper "Efficient Memory Side-Channel Protection for Embedding Generation in Machine Learning".

Description of the sub-directories:

* `EXTENSIONS`
  + Contains various embedding generation techniques as PyTorch extensions.
* `DLRM` application
  + See README for further details. 
* `LLM` application
  + See README for further details
* `MISC`
  + Calculate and print memory footprint of certain models



#### SETUP INSTRUCTIONS

This requires a CPU with a Trusted Execution Environment (TEE); we specifically target Intel CPUs with SGX. 
Hence, we require at least a 3rd Gen (Ice Lake or later) Intel Scalable Xeon Processor with SGX enabled and >64GB secure memory.
We also required AVX-512 support.
SGX further needs Gramine library OS installed (https://gramine.readthedocs.io/en/latest/installation.html) and a signing key generated.

For Python, we use an Anaconda (www.anaconda.com) based flow.
First create a new environment `conda create -n pyth310 python=3.10`.

Then install the following packages:
`pip install torch torchvision torchrec future numpy tqdm onnx pydot scikit-learn tensorboard shapely scipy matplotlib transformers datasets tiktoken wandb tqdm`.

Also install the assembler `conda install anaconda::nasm` and `libssl-dev` or equivalent.

Before running experiments, also install our custom-implemented PyTorch extensions in the `EXTENSIONS` folder.


