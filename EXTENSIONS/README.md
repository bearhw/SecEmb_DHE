Run the following inside each PyTorch extension to install the extension into the proper conda environment.

    source cmd-setup.sh

Note: Use of internal PyTorch ReLU under AVX-512 is likely equivalent to our custom ReLU as the tensor sizes are well-aligned to 64B.