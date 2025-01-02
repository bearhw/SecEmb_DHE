from setuptools import setup
from torch.utils.cpp_extension import BuildExtension, CppExtension

setup(
    name='Argmax_Ext',
    ext_modules=[
        CppExtension(name='Argmax_Ext', sources=['Argmax_Ext.cpp'], extra_compile_args=['-O3','-march=native']),
    ],
    cmdclass={
        'build_ext': BuildExtension
    })


