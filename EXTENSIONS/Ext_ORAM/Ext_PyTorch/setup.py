from setuptools import setup
from torch.utils.cpp_extension import BuildExtension, CppExtension


setup(
    name='EmbeddingBagExt_ZT_PORAM',
    ext_modules=[
        CppExtension('EmbeddingBagExt_ZT_PORAM', sources=['EmbeddingBagExtZT.cpp','/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App/utils.cpp'], extra_link_args=['-Wl,--verbose','-L=/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App','-Wl,--rpath=/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App','-Wl,--sysroot=/','-lZT','-lcrypto'], extra_compile_args=['-O3','-march=native','-DFLAG_ZT_PO','-DFLAG_NO_SETWEIGHT','-I/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App/','-L=/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App']),
    ],
    cmdclass={
        'build_ext': BuildExtension
    })



setup(
    name='EmbeddingBagExt_ZT_CORAM',
    ext_modules=[
        CppExtension('EmbeddingBagExt_ZT_CORAM', sources=['EmbeddingBagExtZT.cpp','/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App/utils.cpp'], extra_link_args=['-Wl,--verbose','-L=/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App','-Wl,--rpath=/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App','-Wl,--sysroot=/','-lZT','-lcrypto'], extra_compile_args=['-O3','-march=native','-DFLAG_ZT_CO','-DFLAG_NO_SETWEIGHT','-I/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App/','-L=/localdata/umar/home/HPCA_ARTIFACT/EXTENSIONS/Ext_ORAM/ZT_Clean/Sample_App']),
    ],
    cmdclass={
        'build_ext': BuildExtension
    })

