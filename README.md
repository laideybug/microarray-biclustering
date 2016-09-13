# Distributed Microarray Biclustering Implementation for Parallella-16 Microserver #

This project is a distributed dictionary learning.

This project contains the following components:

    data/
        data.txt

    include/
        common.h
        config.h
        e_sync.h

    libs/
        e-lib-extended/
        include/
        mb-utils/

    src/
        e_mb.c
        e_mb_batch.c
        e_mb_batch_master.c
        e_mb_master.c
        e_mb_multi.c
        e_mb_multi_master.c
        e_sync.c
        microarray_biclustering.c

    build.sh
    run.sh
    plot.sh

### Building the project ###

At the command prompt, type:

$ ./build.sh

### Running the project ###

At the command prompt, type:

$ ./run.sh

### Plotting the output ###

Plotting the output requires Gnuplot installed so get it with "apt-get install gnuplot".

At the command prompt, type:

$ ./plot.sh