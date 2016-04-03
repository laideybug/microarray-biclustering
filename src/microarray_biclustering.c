#include <e-hal.h>
#include <e-loader.h>
#include <stdio.h>
#include <time.h>
#include "common.h"
#include "microarray_biclustering_utils.h"

#define SHM_OFFSET 0x01000000

int main(int argc, char *argv[]) {
    unsigned current_row, current_col, i, all_done, avg_inf_clks, avg_up_clks, total_inf_clks, total_up_clks, clr;
    float input_data[IN_ROWS][IN_COLS], data_point, dictionary_w[IN_ROWS][N], update_wk[IN_ROWS], dual_var[IN_ROWS], secs;
    int t;
    char path[100] = "../data/data.txt";
#ifdef USE_MASTER_NODE
    unsigned masternode_clks;
    int last_t;
#else
    unsigned inf_clks, up_clks;
    float xt[IN_ROWS], xt_k[WK_ROWS];
    int done[M*N], j, k;
#endif

    clr = CLEAR_FLAG;
    // Seed the random number generator
    srand(1);

    printf("\nReading input data...\n");

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

    printf("Initialising network...\n");

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
	e_open(&dev, 0, 0, M, N);
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

     // Load program to the workgroup but do not run yet
    if (e_load_group("e_microarray_biclustering.srec", &dev, 0, 0, M, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering.srec\n");
        return EXIT_FAILURE;
    }

#ifdef USE_MASTER_NODE
    // Open the master node workgroup
    e_epiphany_t dev_master;
	e_open(&dev_master, MASTER_NODE_ROW, MASTER_NODE_COL, 1, 1);
    e_reset_group(&dev_master);

    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, IN_ROWS*IN_COLS*sizeof(float) + MASTER_ADDR_NUM*sizeof(int)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Write input data to shared memory
    e_write(&mbuf, 0, 0, 0x0, &input_data, IN_ROWS*IN_COLS*sizeof(float));
    // Clear done flag in shared memory
    e_write(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &clr, sizeof(int));

    // Load program to the master core but do not run yet
    if (e_load("e_microarray_biclustering_master.srec", &dev_master, 0, 0, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering_master.srec\n");
        return EXIT_FAILURE;
    }

    last_t = -1;
    secs = 0.0f;

    // Start/wake workgroup
    e_start_group(&dev_master);
    e_start_group(&dev);

    printf("Network started...\n\n");

    while (1) {
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &all_done, sizeof(unsigned));
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float) + sizeof(unsigned), &t, sizeof(int));
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float) + (2*sizeof(unsigned)), &total_inf_clks, sizeof(unsigned));
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float) + (3*sizeof(unsigned)), &total_up_clks, sizeof(unsigned));
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float) + (4*sizeof(unsigned)), &masternode_clks, sizeof(unsigned));

        if (t - last_t) {
            avg_inf_clks = (unsigned)(total_inf_clks * ONE_OVER_M_N);
            avg_up_clks = (unsigned)(total_up_clks * ONE_OVER_M_N);
            last_t = t;
            secs += masternode_clks * ONE_OVER_E_CYCLES;

            printf("\nConfiguration: Master Node\n");
            printf("-------------------------------\n");
            printf("Processed input sample: %i\n", t);
            printf("Average clock cycles for inference step: %i clock cycles\n", avg_inf_clks);
            printf("Average network speed of inference step: %.6f seconds\n", avg_inf_clks * ONE_OVER_E_CYCLES);
            printf("Average clock cycles for update step: %i clock cycles\n", avg_up_clks);
            printf("Average network speed of update step: %.6f seconds\n", avg_up_clks * ONE_OVER_E_CYCLES);
            printf("-------------------------------\n");
            printf("Percent complete: %.2f%%\n", (t+1)*100.0f*ONE_OVER_IN_COLS);
            printf("Average speed: %.2f seconds/sample\n", secs/(t+1));
            printf("Time elapsed: %.2f seconds\n", secs);
            printf("Total time estimate: %.2f seconds\n", (secs/(t+1))*IN_COLS);
            printf("Remaining time estimate: %.2f seconds\n\n", (secs/(t+1))*IN_COLS - secs);
        }

        if (all_done == 1) {
            printf("Done.");
            break;
        }
    }

    e_close(&dev_master);

#else
    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, 3*M*N*sizeof(unsigned)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Load program to the workgroup but do not run yet
    if (e_load_group("e_microarray_biclustering.srec", &dev, 0, 0, M, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering.srec\n");
        return EXIT_FAILURE;
    }

    printf("Network started...\n\n");

    clock_t start = clock(), diff;

    for (t = 0; t < IN_COLS; ++t) {
        getColumn(IN_ROWS, IN_COLS, t, input_data, xt);

        for (j = 0; j < M; ++j) {
            for (k = 0; k < WK_ROWS; ++k) {
                xt_k[k] = *(xt + (k + j*WK_ROWS));
            }

            for (k = 0; k < N; ++k) {
                // "Stream" next data sample
                e_write(&dev, j, k, XT_MEM_ADDR, &xt_k, WK_ROWS*sizeof(float));
                // Clear done flag
                e_write(&mbuf, 0, 0, (j * N + k)*sizeof(unsigned), &clr, sizeof(unsigned));
            }
        }

        // Start/wake workgroup
        e_start_group(&dev);

        while (1) {
            all_done = 0;

            for (j = 0; j < M*N; ++j) {
                e_read(&mbuf, 0, 0, j*sizeof(int), &done[j], sizeof(int));
                all_done += done[j];
            }

            if (all_done == M*N) {
                break;
            }
        }

        total_inf_clks = 0;
        total_up_clks = 0;

        for (j = 0; j < M*N; ++j) {
            e_read(&mbuf, 0, 0, j*sizeof(unsigned) + (M*N*sizeof(unsigned)), &inf_clks, sizeof(unsigned));
            total_inf_clks += inf_clks;
            e_read(&mbuf, 0, 0, j*sizeof(unsigned) + (2*M*N*sizeof(unsigned)), &up_clks, sizeof(unsigned));
            total_up_clks += up_clks;
        }

        avg_inf_clks = (unsigned)(total_inf_clks * ONE_OVER_M_N);
        avg_up_clks = (unsigned)(total_up_clks * ONE_OVER_M_N);

        diff = clock() - start;
        secs = diff / CLOCKS_PER_SEC;

        printf("\nConfiguration: ARM\n");
        printf("-------------------------------\n");
        printf("Processed input sample: %i\n", t);
        printf("Average clock cycles for inference step: %i clock cycles\n", avg_inf_clks);
        printf("Average network speed of inference step: %.6f seconds\n", avg_inf_clks * ONE_OVER_E_CYCLES);
        printf("Average clock cycles for update step: %i clock cycles\n", avg_up_clks);
        printf("Average network speed of update step: %.6f seconds\n", avg_up_clks * ONE_OVER_E_CYCLES);
        printf("-------------------------------\n");
        printf("Percent complete: %.2f%%\n", (t+1)*100.0f*ONE_OVER_IN_COLS);
        printf("Average speed: %.2f seconds/sample\n", secs/(t+1));
        printf("Time elapsed: %.2f seconds\n", secs);
        printf("Total time: %.2f seconds\n", (secs/(t+1))*IN_COLS);
        printf("Remaining time: %.2f seconds\n\n", (secs/(t+1))*IN_COLS - secs);
    }

    printf("Done.");
#endif

    e_close(&dev);
    e_free(&mbuf);
    e_finalize();

    return EXIT_SUCCESS;
}
