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
${CROSS_PREFIX}gcc -Wall -std=c99 -O3 src/microarray_biclustering.c -o bin/microarray_biclustering.elf ${EINCS} ${ELIBS} ${INCS} ${LIBS} -le-hal -le-loader -lm -lmb-utils -lgnuplot-i

# Build agent DEVICE side programs
e-gcc -Wall -std=c99 -Os -funroll-loops -falign-loops=8 -falign-functions=8 -ffast-math -ffp-contract=fast -mfp-mode=round-nearest -T ${ELDF} src/e_mb.c src/e_synch.c -o bin/e_mb.elf ${INCS} ${LIBS} -lm -le-lib -le-lib-extended
e-gcc -Wall -std=c99 -Os -funroll-loops -falign-loops=8 -falign-functions=8 -ffast-math -ffp-contract=fast -mfp-mode=round-nearest -T ${ELDF} src/e_mb_batch.c src/e_synch.c -o bin/e_mb_batch.elf ${INCS} ${LIBS} -lm -le-lib -le-lib-extended
e-gcc -Wall -std=c99 -Os -funroll-loops -falign-loops=8 -falign-functions=8 -ffast-math -ffp-contract=fast -mfp-mode=round-nearest -T ${ELDF} src/e_mb_multi.c src/e_synch.c -o bin/e_mb_multi.elf ${INCS} ${LIBS} -lm -le-lib -le-lib-extended

# Convert agent ebinaries to SREC files
e-objcopy --srec-forceS3 --output-target srec bin/e_mb.elf bin/e_mb.srec
e-objcopy --srec-forceS3 --output-target srec bin/e_mb_batch.elf bin/e_mb_batch.srec
e-objcopy --srec-forceS3 --output-target srec bin/e_mb_multi.elf bin/e_mb_multi.srec

# Build master DEVICE side programs
e-gcc -Wall -std=c99 -Os -funroll-loops -falign-loops=8 -falign-functions=8 -ffast-math -ffp-contract=fast -mfp-mode=round-nearest -T ${ELDF} src/e_mb_master.c -o bin/e_mb_master.elf -le-lib -le-lib-extended
e-gcc -Wall -std=c99 -Os -funroll-loops -falign-loops=8 -falign-functions=8 -ffast-math -ffp-contract=fast -mfp-mode=round-nearest -T ${ELDF} src/e_mb_batch_master.c -o bin/e_mb_batch_master.elf -le-lib -le-lib-extended
e-gcc -Wall -std=c99 -Os -funroll-loops -falign-loops=8 -falign-functions=8 -ffast-math -ffp-contract=fast -mfp-mode=round-nearest -T ${ELDF} src/e_mb_multi_master.c -o bin/e_mb_multi_master.elf -le-lib -le-lib-extended

# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec bin/e_mb_master.elf bin/e_mb_master.srec
e-objcopy --srec-forceS3 --output-target srec bin/e_mb_batch_master.elf bin/e_mb_batch_master.srec
e-objcopy --srec-forceS3 --output-target srec bin/e_mb_multi_master.elf bin/e_mb_multi_master.srec