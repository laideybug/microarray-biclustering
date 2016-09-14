# Multicore Distributed Dictionary Learning: A Microarray Gene Expression Biclustering Case Study #

This project is an experimental study of a distributed machine learning technique on a multi-core platform. The project was developed for:

* Epiphany E16G301 many-core architecture
* Epiphany SDK v2015.1

### Introduction ###

Current machine learning algorithms are highly centralised, with a large number of processing agents, distributed across parallel processing resources, accessing a single, very large data object. This creates bottlenecks as a result of limited memory access rates. Distributed learning has the potential to resolve this problem by employing networks of co-operating agents each operating on subsets of the data, but as yet their suitability for realisation on parallel architectures such as multicore are unknown. This project presents the deployment of a distributed dictionary learning algorithm for microarray gene expression bi-clustering on a 16-core Epiphany multicore.

### Overview ###

In typical centralised dictionary learning algorithms, each *agent* in a network maintains a copy of the entire dictionary. In this approach, however, each utilised core of the Epiphany (eCore) is only in charge of a single dictionary column. The networked eCores work co-operatively to learn the entire dictionary in a distributed fashion. The input dataset for this experiment consists of 12,625 gene expression levels taken from a sample of 56 subjects. The subjects are classified into four lung cancer categories:

* **Normal**: Subjects without cancer
* **Carcinoid**: Patients with pulmonary carcinoid tumours
* **Colon**: Patients with colon metastases
* **SmallCell**: Patients with small-cell carcinoma

The data is presented as a 56 x 12,625 matrix found in *data.txt*. The rows represent the patients grouped together by their classification, while the columns represent their corresponding gene expression levels.

This experiment is an unsupervised machine learning task in that the eCores are unaware of the ground truth of the classification of each patient. The ultimate aim, however, is for the eCores to group patients with similar genetic information into *clusters*. Once the process is completed, different coloured markers are applied to each subject for identification and evaluation on the plotted graph. The algorithm is a decentralised *online* algorithm in that the eCores are exposed to each column of the cancer data matrix once in a streaming manner.

### Modes ###

Two fully-realised implementations are included in the project - **Distributed** and **Batch-distributed**. A third, **incomplete** implementation is also included, called **Multi-distributed**.

* The *distributed* implementation separates the randomly initialised dictionary *W* into three atoms, one per column, with the learning process associated with each atom undertaken by a single eCore.

* The *batch-distributed* realisation builds on the distributed version. In this case the dictionary *W* is again subdivided into three atoms but multiple input samples are processed in batch mode. In this case, a (4, 3) workgroup is used, with element (i, j) hosting atom *W*j . Specifically - each row of the workgroup processes atoms from a single sub-group as previously, but in this case four rows are used to process four input samples concurrently.

* The *multi-distributed* version attempts to expand the *distributed* technique across twelve cores instead of three in order to reduce the amount of work each core is required to perform for each input sample. Work on this implementation is ongoing.

Switch between modes by altering the *config.h* file and rebuilding the project before running.

### Project structure ###

This project contains the following components:

    data/                - where input and output data are stored
    
    include/             - the project header files
    
    libs/                - where additional project libraries are stored
        e-lib-extended/  - a custom library that extends a number of the eSDK functions
        include/         - the external library header files
        mb-utils/        - a custom math library for working with the vectors in the project
    
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