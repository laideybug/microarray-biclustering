#include <e-hal.h>
#include <e-loader.h>
#include <stdio.h>
#include <time.h>
#include "common.h"
#include "microarray_biclustering_utils.h"

#define SHM_OFFSET 0x01000000

int main(int argc, char *argv[]) {
    // Read input data
    float input_data[IN_ROWS][IN_COLS], data_point, dictionary_w[IN_ROWS][N], update_wk[IN_ROWS], dual_var[IN_ROWS];
    int current_row, current_col, i, clr, done;
    char path[100] = "../../data/data.txt";

    // Seed the random number generator
    srand(1);

    // Open data.txt
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

    // Open a 1 x N+1 workgroup
	e_open(&dev, 0, 0, 1, N+1);
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

    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, IN_ROWS*IN_COLS*sizeof(float) + sizeof(int)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Write input data to shared memory
    e_write(&mbuf, 0, 0, 0x0, &input_data, IN_ROWS*IN_COLS*sizeof(float));
    // Clear done flag in shared memory
    clr = 0x00000000;
    e_write(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &clr, sizeof(int));

    // Load program to the workgroup but do not run yet
    if (e_load_group("e_microarray_biclustering.srec", &dev, 0, 0, 1, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering.srec\n");
        return EXIT_FAILURE;
    }

    // Load program to the master core but do not run yet
    if (e_load("e_microarray_biclustering_master.srec", &dev, 0, MASTER_COL, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering_master.srec\n");
        return EXIT_FAILURE;
    }

    // Start/wake workgroup
    e_start_group(&dev);

    while (1) {
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &done, sizeof(int));

        if (done == 1) {
            break;
        }
    }

    e_close(&dev);
    e_free(&mbuf);
    e_finalize();

	return EXIT_SUCCESS;
}
