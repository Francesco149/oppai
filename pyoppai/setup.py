import sys
from distutils.core import setup, Extension

pyoppai = Extension(
    'pyoppai',
    define_macros = [
        ('OPPAI_LIB', '1'),
        #('OPPAI_MODULE_DEBUG', '1')
    ],
    libraries = [
        'Advapi32.lib' if "win" in sys.platform else 'crypto'
    ],
    sources = ['pyoppaimodule.cc']
)

setup(
    name = 'pyoppai',
    version = '0.9.2-b1.0',
    description = 'Python bindings for the oppai osu! pp calculator',
    ext_modules = [pyoppai]
)
