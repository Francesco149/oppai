import sys
from distutils.core import setup, Extension

pyoppai = Extension(
    'pyoppai',

    define_macros = [
        ('OPPAI_LIB', '1'),
        ('OPPAI_SSIZE_T_DEFINED', '1'),
        ('_CRT_SECURE_NO_WARNINGS', '1'),
        ('NOMINMAX', '1')
    ] if "win" in sys.platform else [
        ('OPPAI_LIB', '1'),
        #('OPPAI_MODULE_DEBUG', '1'),
    ],

    extra_compile_args = [
        "-O2",
        "-nologo", "-MT", "-Gm-", "GR-", "-EHsc", "-W4", #"-WX",
        "-wd4201", "-wd4100", "-F8000000"
    ] if "win" in sys.platform else [
        "-O2",
        "-Wno-variadic-macros",
        "-Wall", #"-Werror" #TODO: fix warnings and use -Werror?
    ],

    libraries = [
        'Advapi32' if "win" in sys.platform else 'crypto'
    ],

    sources = ['pyoppaimodule.cc']
)

setup(
    name = 'pyoppai',
    version = '0.9.3-b1.1',
    description = 'Python bindings for the oppai osu! pp calculator',
    ext_modules = [pyoppai]
)
