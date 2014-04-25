#setup.py
from distutils.core import setup, Extension
from distutils.sysconfig import get_python_version

import sys

# NRNPYTHON_DEFINES which were enabled at configure time
extern_defines = ""
nrnpython_exec = ""
nrnpython_pyver = ""
nrn_srcdir = "."
ivlibdir = ""
if ivlibdir == "" :
    ivlibdir = '.'

instdir = "/Users/stephan/Code/git/neuron/nrn-7.3"
if nrn_srcdir[0] != '/' :
    nrn_srcdir = '../../' + nrn_srcdir

if nrnpython_pyver!=get_python_version():
    print "Error:"
    print "NEURON configure time python: "+nrnpython_exec+"  "+ nrnpython_pyver
    print "Python presently executing setup.py: "+sys.executable+"   "+ get_python_version()
    print "These do not match, and they should!"
    sys.exit(1)


ldefs = extern_defines.split('-D')

# if using MPI then at least for linking need special paths and libraries
mpicc_bin = "gcc-mp-4.8"
mpicxx_bin = "g++-mp-4.8"
import os
os.environ["CC"]=mpicc_bin
os.environ["CXX"]=mpicxx_bin

# apparently we do not need the following
#################################
## following http://code.google.com/p/maroonmpi/wiki/Installation
## hack into distutils to replace the compiler in "linker_so" with mpicxx_bin
#
#import distutils
#import distutils.unixccompiler
#
#class MPI_UnixCCompiler(distutils.unixccompiler.UnixCCompiler):
#    __set_executable = distutils.unixccompiler.UnixCCompiler.set_executable
#
#    def set_executable(self,key,value):
#	print "MPI_UnixCCompiler ", key, " | ", value
#        if key == 'linker_so' and type(value) == str:
#            value = mpicxx_bin + ' ' + ' '.join(value.split()[1:])
#
#        return self.__set_executable(key,value)
#    
#distutils.unixccompiler.UnixCCompiler = MPI_UnixCCompiler
#################################

include_dirs = [nrn_srcdir+'/src/oc', '../oc', nrn_srcdir+'/src/nrnmpi']
defines = []

libdirs = ["/Users/stephan/Code/git/neuron/nrn-7.3/x86_64/lib",
  ivlibdir
]
epre='-Wl,-R'



hoc_module = Extension(
      "neuron.hoc",
      ["inithoc.cpp"],
      library_dirs=libdirs,
       = [ epre+libdirs[0],epre+libdirs[1] ],
      #extra_objects = [],
      libraries = [
	"nrnpython",
        "nrnoc", "oc", "nrniv", "ivoc",
        "memacs", "meschach", "neuron_gnu",
	"nrnmpi",
        "scopmath", "sparse13", "sundials", "ivos",
	"readline"
      ],
      include_dirs = include_dirs,
      define_macros=defines
    )

# specify that the data_files paths are relative to same place as python files
# from http://stackoverflow.com/questions/1612733/including-non-python-files-with-setup-py
from distutils.command.install import INSTALL_SCHEMES
for scheme in INSTALL_SCHEMES.values():
    scheme['data'] = scheme['purelib']

setup(name="NEURON", version="7.3",
      description = "NEURON bindings for python",
      package_dir = {'':instdir+'/share/nrn/lib/python'},
      packages=['neuron','neuron.neuroml','neuron.tests', 'neuron.rxd', 'neuron.rxd.geometry3d'],
      data_files = [('neuron', [nrn_srcdir + '/share/lib/python/neuron/help_data.dat'])],
      ext_modules=[hoc_module]
)

