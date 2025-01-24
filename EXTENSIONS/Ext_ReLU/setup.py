from setuptools import setup
from torch.utils.cpp_extension import BuildExtension, CppExtension

setup(
    name='ReLU_Ext',
    ext_modules=[
        CppExtension(name='ReLU_Ext', sources=['ReLU_Ext.cpp'], extra_compile_args=['-O3','-DFLAG_RELU_OBLIVIOUS','-march=native','-DAT_PARALLEL_OPENMP', '-fopenmp']),
    ],
    cmdclass={
        'build_ext': BuildExtension
    })

# FLAG_RELU_OBLIVIOUS
# FLAG_RELU_NONSECURE