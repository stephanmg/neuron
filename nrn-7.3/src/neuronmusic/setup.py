#setup.py for neuronmusic extension
from distutils.core import setup, Extension

nrn_srcdir = "."
instdir = "/home/stephan/code/hg/nrn/nrn-7.3"
if nrn_srcdir[0] != '/' :
    nrn_srcdir = '../../' + nrn_srcdir

mpicc_bin = "gcc"
mpicxx_bin = "g++"
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
    runtime_library_dirs = ['/home/stephan/code/hg/nrn/nrn-7.3/i686/lib', ''],
    extra_objects = ['/home/stephan/code/hg/nrn/nrn-7.3/i686/lib/libnrniv.so.0',
                     '/libIVhines.so.3' \
                     if len ('') else ''],
    include_dirs = include_dirs
)

setup(name="neuronmusic", version="7.3",
    description = "NEURON+MUSIC python interface",
    package_dir = {'':instdir+'/share/nrn/lib/python'},
    ext_modules=[neuronmusic_module]
)
