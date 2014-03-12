#!/bin/sh

prefix=/home/stephan/hg/nrn/nrn-7.3
exec_prefix=/home/stephan/hg/nrn/nrn-7.3/x86_64
NRNBIN=${exec_prefix}/bin
ARCH=x86_64
NEURONHOME=/home/stephan/hg/nrn/nrn-7.3/share/nrn

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

