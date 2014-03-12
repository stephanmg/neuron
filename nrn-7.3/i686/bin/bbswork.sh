#!/bin/sh

prefix=/home/stephan/code/hg/nrn/nrn-7.3
exec_prefix=/home/stephan/code/hg/nrn/nrn-7.3/i686
NRNBIN=${exec_prefix}/bin
ARCH=i686
NEURONHOME=/home/stephan/code/hg/nrn/nrn-7.3/share/nrn

cd $1

if [ -x ${ARCH}/special ] ; then
	program="./${ARCH}/special"
else
	program="${NRNBIN}/nrniv"
fi

hostname
pwd
shift
shift
echo "time $program $*"
time $program $*

