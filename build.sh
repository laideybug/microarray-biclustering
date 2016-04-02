#!/bin/bash

set -e

ESDK=${EPIPHANY_HOME}
ELIBS="-L ${ESDK}/tools/host/lib"
EINCS="-I ${ESDK}/tools/host/include"
ELDF=${ESDK}/bsps/current/internal.ldf

SCRIPT=$(readlink -f "$0")
EXEPATH=$(dirname "$SCRIPT")
cd $EXEPATH

mkdir -p bin

CROSS_PREFIX=
case $(uname -p) in
	arm*)
		# Use native arm compiler (no cross prefix)
		CROSS_PREFIX=
		;;
	   *)
		# Use cross compiler
		CROSS_PREFIX="arm-linux-gnueabihf-"
		;;
esac

# Build HOST side application
${CROSS_PREFIX}gcc -Wall -std=c99 -Os src/microarray_biclustering.c src/microarray_biclustering_utils.c -o bin/microarray_biclustering.elf ${EINCS} ${ELIBS} -le-hal -le-loader -lm

# Build DEVICE side program
e-gcc -Wall -funroll-loops -mfp-mode=round-nearest -Os -T ${ELDF} src/e_microarray_biclustering.c src/e_microarray_biclustering_utils.c src/e_synch.c -o bin/e_microarray_biclustering.elf -le-lib -lm

# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec bin/e_microarray_biclustering.elf bin/e_microarray_biclustering.srec

# Build DEVICE side program
e-gcc -Wall -funroll-loops -mfp-mode=round-nearest -Os -T ${ELDF} src/e_microarray_biclustering_master.c src/e_microarray_biclustering_utils.c -o bin/e_microarray_biclustering_master.elf -le-lib

# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec bin/e_microarray_biclustering_master.elf bin/e_microarray_biclustering_master.srec