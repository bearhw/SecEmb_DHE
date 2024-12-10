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









setup(
    name='EmbeddingBagExt_LinearScan_v9n_llm',
    ext_modules=[
        CppExtension(name='EmbeddingBagExt_LinearScan_v9n_llm', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V9N_LLM','-march=native','-DAT_PARALLEL_OPENMP', '-fopenmp']),
    ],
    cmdclass={
        'build_ext': BuildExtension
    })













# setup(
#     name='EmbeddingBagExt_Index',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_Index', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_INDEX_LOOKUP']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })


# # exit()

# setup(
#     name='EmbeddingBagExt_LinearScan_v1',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v1', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V1']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })


# setup(
#     name='EmbeddingBagExt_LinearScan_v2',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v2', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V2']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })


# setup(
#     name='EmbeddingBagExt_LinearScan_v3',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v3', sources=['EmbeddingBagExt.cpp'], extra_link_args=['/home/umar/CPP_EXT/New1/LSasmTest/oblivfunc.o'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V3']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })


# setup(
#     name='EmbeddingBagExt_LinearScan_v4',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v4', sources=['EmbeddingBagExt.cpp'], extra_link_args=['/home/umar/CPP_EXT/New1/LSasmTest/oblivfunc.o'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V4']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })


# setup(
#     name='EmbeddingBagExt_LinearScan_v5',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v5', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V5']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })

# setup(
#     name='EmbeddingBagExt_LinearScan_v6',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v6', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V6']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })



# setup(
#     name='EmbeddingBagExt_LinearScan_v7',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v7', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V7','-mavx2']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })

# setup(
#     name='EmbeddingBagExt_LinearScan_v8',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v8', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V8','-mavx2']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })




# setup(
#     name='EmbeddingBagExt_LinearScan_v9',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v9', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V9','-march=icelake-server']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })

# setup(
#     name='EmbeddingBagExt_LinearScan_v10',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v10', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V10','-march=icelake-server']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })










# setup(
#     name='EmbeddingBagExt_LinearScan_v5n',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v5n', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V5N','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })








# setup(
#     name='EmbeddingBagExt_LinearScan_v6n',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v6n', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V6N','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })




# setup(
#     name='EmbeddingBagExt_LinearScan_v6nn',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v6nn', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V6NN','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })







# setup(
#     name='EmbeddingBagExt_LinearScan_v7n',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v7n', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V7N','-mavx2','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })

# setup(
#     name='EmbeddingBagExt_LinearScan_v8nn',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v8nn', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V8NN','-mavx2','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })





# setup(
#     name='EmbeddingBagExt_LinearScan_v9nn',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v9nn', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V9NN','-march=icelake-server','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })

# setup(
#     name='EmbeddingBagExt_LinearScan_v10n',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v10n', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V10N','-march=icelake-server','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })




# setup(
#     name='EmbeddingBagExt_LinearScan_v10nn',
#     ext_modules=[
#         CppExtension(name='EmbeddingBagExt_LinearScan_v10nn', sources=['EmbeddingBagExt.cpp'], extra_compile_args=['-O3','-DFLAG_LINEAR_SCAN_V10NN','-march=icelake-server','-DAT_PARALLEL_OPENMP', '-fopenmp']),
#     ],
#     cmdclass={
#         'build_ext': BuildExtension
#     })


