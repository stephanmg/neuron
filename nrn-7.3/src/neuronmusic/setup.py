#setup.py for neuronmusic extension
from distutils.core import setup, Extension

nrn_srcdir = "."
instdir = "/Users/stephan/Code/git/neuron/nrn-7.3"
if nrn_srcdir[0] != '/' :
    nrn_srcdir = '../../' + nrn_srcdir

mpicc_bin = "gcc-mp-4.8"
mpicxx_bin = "g++-mp-4.8"
import os
os.environ["CC"]=mpicc_bin
os.environ["CXX"]=mpicxx_bin

include_dirs = ['.', nrn_srcdir+'/src/neuronmusic', '.']
libdirs = ['.']

neuronmusic_module = Extension(
    'neuronmusic',
    ['neuronmusic.cpp'],
    library_dirs = libdirs,
    libraries = ['music'],
    runtime_library_dirs = ['/Users/stephan/Code/git/neuron/nrn-7.3/x86_64/lib', ''],
    extra_objects = ['/Users/stephan/Code/git/neuron/nrn-7.3/x86_64/lib/libnrniv.so.0',
                     '/libIVhines.so.3' \
                     if len ('') else ''],
    include_dirs = include_dirs
)

setup(name="neuronmusic", version="7.3",
    description = "NEURON+MUSIC python interface",
    package_dir = {'':instdir+'/share/nrn/lib/python'},
    ext_modules=[neuronmusic_module]
)
