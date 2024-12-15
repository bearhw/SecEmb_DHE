from setuptools import setup
from torch.utils.cpp_extension import BuildExtension, CppExtension


setup(
    name='EmbeddingBagExt_LinearScan',
    ext_modules=[
        CppExtension(name='EmbeddingBagExt_LinearScan', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V9N','-march=native','-DAT_PARALLEL_OPENMP', '-fopenmp']),
    ],
    cmdclass={
        'build_ext': BuildExtension
    })
