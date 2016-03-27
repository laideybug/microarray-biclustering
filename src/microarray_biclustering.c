#include <e-hal.h>
#include <e-loader.h>
#include <stdio.h>
#include <time.h>
#include "common.h"
#include "microarray_biclustering_utils.h"

#define SHM_OFFSET 0x01000000

int main(int argc, char *argv[]) {
    float input_data[IN_ROWS][IN_COLS], data_point, dictionary_w[IN_ROWS][N], update_wk[IN_ROWS], dual_var[IN_ROWS];
    int current_row, current_col, i, all_done, clr = 0x00000000;
    char path[100] = "../data/data.txt";
#if !USE_MASTER_NODE
    int done[N], j;
    float xt[IN_ROWS];
#endif

    // Seed the random number generator
    srand(1);

    // Read input data
    FILE *file;
    file = fopen(path, "r");

    if (file != NULL) {
        current_row = 0;
        current_col = 0;

        while (fscanf(file, "%f", &data_point) != EOF) {
            input_data[current_row][current_col] = data_point;

            // data.txt is in 12625 x 56 format,
            // therefore we need to model it as
            // a 56 x 12625 matrix
            if (current_row < (IN_ROWS - 1)) {
                current_row++;
            } else {
                current_row = 0;
                current_col++;
            }
        }

        fclose(file);
    } else {
        printf("Error: Failed to open input file.\n");
        return EXIT_FAILURE;
    }

    // Remove DC component from input data (i.e. centering)
    removeDC(IN_ROWS, IN_COLS, input_data);
    // Initialise the dictionaries
    initDictionary(IN_ROWS, N, dictionary_w);

    // Epiphany setup
    e_platform_t platform;
	e_epiphany_t dev;
	e_mem_t mbuf;

    e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);

	e_set_loader_verbosity(L_D0);
	e_set_host_verbosity(H_D0);

    // Open the workgroup
	e_open(&dev, 0, 0, 1, N + USE_MASTER_NODE);
    e_reset_group(&dev);

    // Initialise update dictionary and dual variable vectors with 0
    fillVector(IN_ROWS, update_wk, 0.0f);
    fillVector(IN_ROWS, dual_var, 0.0f);

    // Load the dictionary atoms into each core
    for (i = 0; i < N; ++i) {
        float dictionary_wk[IN_ROWS];
        getColumn(IN_ROWS, N, i, dictionary_w, dictionary_wk);

        e_write(&dev, 0, i, WK_MEM_ADDR, &dictionary_wk, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, UP_WK_MEM_ADDR, &update_wk, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_OPT_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_K0_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_K1_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
        e_write(&dev, 0, i, NU_K2_MEM_ADDR, &dual_var, IN_ROWS*sizeof(float));
    }

#if USE_MASTER_NODE
    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, IN_ROWS*IN_COLS*sizeof(float) + sizeof(int)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Write input data to shared memory
    e_write(&mbuf, 0, 0, 0x0, &input_data, IN_ROWS*IN_COLS*sizeof(float));
    // Clear done flag in shared memory
    e_write(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &clr, sizeof(int));
#else
    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, N*sizeof(int)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };
#endif

    // Load program to the workgroup but do not run yet
    if (e_load_group("e_microarray_biclustering.srec", &dev, 0, 0, 1, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering.srec\n");
        return EXIT_FAILURE;
    }

#if USE_MASTER_NODE
    // Load program to the master core but do not run yet
    if (e_load("e_microarray_biclustering_master.srec", &dev, 0, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering_master.srec\n");
        return EXIT_FAILURE;
    }

    // Start/wake workgroup
    e_start_group(&dev);

    while (1) {
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &all_done, sizeof(int));

        if (all_done == 1) {
            break;
        }
    }
#else
    clock_t start = clock(), diff;

    for (i = 0; i < IN_COLS; ++i) {
        getColumn(IN_ROWS, IN_COLS, i, input_data, xt);

        for (j = 0; j < N; ++j) {
            e_write(&dev, 0, j, XT_MEM_ADDR, &xt, IN_ROWS*sizeof(float));   // "Stream" next data sample
            e_write(&mbuf, 0, 0, j*sizeof(int), &clr, sizeof(int));  // Clear done flag
        }

        // Start/wake workgroup
        e_start_group(&dev);
        printf("Processing input sample %i...\n\n", i);

        while (1) {
            all_done = 0;

            for (j = 0; j < N; ++j) {
                e_read(&mbuf, 0, 0, j*sizeof(int), &done[j], sizeof(int));
                all_done += done[j];
            }

            if (all_done == N) {
                break;
            }
        }

        diff = clock() - start;

        float secs = diff / CLOCKS_PER_SEC;
        printf("Percent complete: %.3f%%, Average speed: %.2f seconds/sample\nTime elapsed: %.2f seconds Total time: %.2f seconds, Remaining time: %.2f seconds\n\n", (i+1)*100.0f/IN_COLS, secs/(i+1), secs, (secs/(i+1))*IN_COLS, (secs/(i+1))*IN_COLS - secs);
    }
#endif

    e_close(&dev);
    e_free(&mbuf);
    e_finalize();

    return EXIT_SUCCESS;
}
