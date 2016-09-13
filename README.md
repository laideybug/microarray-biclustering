# Distributed Dictionary Learning Implementation for Parallella-16 Microserver #

This project is a distributed dictionary learning. The subjects are classified into four lung cancer categories:

* *Normal*: Subjects without cancer
* *Carcinoid*: Patients with pulmonary carcinoid tumours
* *Colon*: Patients with colon metastases
* *SmallCell*: Patients with small-cell carcinoma

This project contains the following components:

    data/                - where input and output data are stored
    include/             - the project header files
    libs/                - where additional project libraries are stored
    src/                 - the source files

    build.sh             - shell script for building the project
    run.sh               - shell script for running the project
    plot.sh              - shell script for plotting the learned dictionary with Gnuplot

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