#!/bin/bash

set -e

SCRIPT=$(readlink -f "$0")
EXEPATH=$(dirname "$SCRIPT")

LIBS="-L ${EXEPATH}/libs"
INCS="-I ${EXEPATH}/libs/include -I ${EXEPATH}/include"

ESDK=${EPIPHANY_HOME}
ELIBS="-L ${ESDK}/tools/host/lib"
EINCS="-I ${ESDK}/tools/host/include"
ELDF=${ESDK}/bsps/current/internal.ldf

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

# Build LIBRARIES
cd libs

${CROSS_PREFIX}gcc -Wall -std=c99 -O3 -c mb-utils/*.c ${INCS} -lm
ar rs libmb-utils.a *.o
rm -rf *.o

${CROSS_PREFIX}gcc -O3 -c gnuplot-i/*.c ${INCS}
ar rs libgnuplot-i.a *.o
rm -rf *.o

e-gcc -Wall -std=c99 -O3 -c e-lib-extended/*.c ${INCS} -le-lib
ar rs libe-lib-extended.a *.o
rm -rf *.o

cd ..

# Build HOST side application
${CROSS_PREFIX}gcc -Wall -std=c99 -Os src/microarray_biclustering.c -o bin/microarray_biclustering.elf ${EINCS} ${ELIBS} ${INCS} ${LIBS} -le-hal -le-loader -lmb-utils -lgnuplot-i -lm

# Build DEVICE side program
e-gcc -Wall -funroll-loops -mfp-mode=round-nearest -Os -T ${ELDF} src/e_microarray_biclustering.c src/e_synch.c -o bin/e_microarray_biclustering.elf ${INCS} ${LIBS} -le-lib -le-lib-extended -lm

# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec bin/e_microarray_biclustering.elf bin/e_microarray_biclustering.srec

# Build DEVICE side program
e-gcc -Wall -funroll-loops -mfp-mode=round-nearest -Os -T ${ELDF} src/e_microarray_biclustering_master.c -o bin/e_microarray_biclustering_master.elf ${INCS} ${LIBS} -le-lib -le-lib-extended

# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec bin/e_microarray_biclustering_master.elf bin/e_microarray_biclustering_master.srec