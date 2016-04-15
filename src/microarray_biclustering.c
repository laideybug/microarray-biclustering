#include <e-hal.h>
#include <e-loader.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "microarray_biclustering_utils.h"

#define SHM_OFFSET 0x01000000

int main(int argc, char *argv[]) {
    unsigned current_row, current_col, j, k, avg_inf_clks, avg_up_clks, total_inf_clks, total_up_clks, clr;
    float input_data[IN_ROWS][IN_COLS], dictionary_w[IN_ROWS][N], update_wk[IN_ROWS], dual_var[IN_ROWS], data_point, secs;
    int t;
    char path[100] = "../data/data.txt";
#ifdef USE_MASTER_NODE
    unsigned masternode_clks, all_done;
    int previous_t;
#else
    unsigned done[M_N], inf_clks, up_clks, all_done;
    float xt[IN_ROWS];
    int last_sample;
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
    for (j = 0; j < M; ++j) {
        for (k = 0; k < N; ++k) {
            float dictionary_wk[IN_ROWS];
            getColumn(IN_ROWS, N, k, dictionary_w, dictionary_wk);

            e_write(&dev, j, k, WK_MEM_ADDR, &dictionary_wk, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, UP_WK_MEM_ADDR, &update_wk, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K0_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K1_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K2_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K3_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_K0_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_K1_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_K2_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
        }
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
    if (e_alloc(&mbuf, SHM_OFFSET, IN_ROWS*IN_COLS*sizeof(float) + MASTER_ADDR_NUM*sizeof(unsigned)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Write input data to shared memory
    e_write(&mbuf, 0, 0, 0x0, &input_data, IN_ROWS*IN_COLS*sizeof(float));
    // Clear done flag in shared memory
    e_write(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &clr, sizeof(unsigned));

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

            printf("\nConfiguration: Master Node - %i x %i\n", M, N);
            printf("---------------------------------------\n");
            printf("Processed input sample: %u\n", t);
            printf("Average clock cycles for inference step: %u clock cycles\n", avg_inf_clks);
            printf("Average network speed of inference step: %.6f seconds\n", avg_inf_clks * ONE_OVER_E_CYCLES);
            printf("Average clock cycles for update step: %u clock cycles\n", avg_up_clks);
            printf("Average network speed of update step: %.6f seconds\n", avg_up_clks * ONE_OVER_E_CYCLES);
            printf("Master node clock cycles: %u clock cycles\n", masternode_clks);
            printf("-------------------------------\n");
            printf("Percent complete: %.2f%%\n", (t+1)*100.0f*ONE_OVER_IN_COLS);
            printf("Average speed: %.2f seconds/sample\n", secs/(t+1));
            printf("Time elapsed: %.2f seconds\n", secs);
            printf("Total time estimate: %.2f seconds\n", (secs/(t+1))*IN_COLS);
            printf("Remaining time estimate: %.2f seconds\n\n", (secs/(t+1))*IN_COLS - secs);
        }

        if (all_done == 1) {
            break;
        }
    }

    e_close(&dev_master);

#else
    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, 3*M_N*sizeof(unsigned)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Load program to the workgroup but do not run yet
    if (e_load_group("e_microarray_biclustering.srec", &dev, 0, 0, M, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load e_microarray_biclustering.srec\n");
        return EXIT_FAILURE;
    }

    printf("Network started...\n\n");

    last_sample = 0;
    clock_t start = clock(), diff;

    for (t = 0; t < IN_COLS; t+=M) {
        if (IN_COLS - t == 1) last_sample = 0;

        for (j = 0; j < M; ++j) {
            getColumn(IN_ROWS, IN_COLS, t+j*last_sample, input_data, xt);

            for (k = 0; k < N; ++k) {
                e_write(&dev, j, k, XT_MEM_ADDR, &xt, IN_ROWS*sizeof(float));   // "Stream" next data sample
                e_write(&mbuf, 0, 0, (j*N+k)*sizeof(unsigned), &clr, sizeof(unsigned));  // Clear done flag
            }
        }

        // Start/wake workgroup
        e_start_group(&dev);

        while (1) {
            all_done = 0;

            for (j = 0; j < M_N; ++j) {
                e_read(&mbuf, 0, 0, j*sizeof(unsigned), &done[j], sizeof(unsigned));
                all_done += done[j];
            }

            if (all_done == M_N) {
                break;
            }
        }

        total_inf_clks = 0;
        total_up_clks = 0;

        for (j = 0; j < M_N; ++j) {
            e_read(&mbuf, 0, 0, j*sizeof(unsigned) + (M_N*sizeof(unsigned)), &inf_clks, sizeof(unsigned));
            total_inf_clks += inf_clks;
            e_read(&mbuf, 0, 0, j*sizeof(unsigned) + (2*M_N*sizeof(unsigned)), &up_clks, sizeof(unsigned));
            total_up_clks += up_clks;
        }

        avg_inf_clks = (unsigned)(total_inf_clks * ONE_OVER_M_N);
        avg_up_clks = (unsigned)(total_up_clks * ONE_OVER_M_N);

        diff = clock() - start;
        secs = diff / CLOCKS_PER_SEC;

        printf("\nConfiguration: ARM - %i x %i\n", M, N);
        printf("-------------------------------\n");
        printf("Processed input sample: %u\n", t);
        printf("Average clock cycles for inference step: %u clock cycles\n", avg_inf_clks);
        printf("Average network speed of inference step: %.6f seconds\n", avg_inf_clks * ONE_OVER_E_CYCLES);
        printf("Average clock cycles for update step: %u clock cycles\n", avg_up_clks);
        printf("Average network speed of update step: %.6f seconds\n", avg_up_clks * ONE_OVER_E_CYCLES);
        printf("-------------------------------\n");
        printf("Percent complete: %.2f%%\n", (t+1)*100.0f*ONE_OVER_IN_COLS);
        printf("Average speed: %.2f seconds/sample\n", secs/(t+1));
        printf("Time elapsed: %.2f seconds\n", secs);
        printf("Total time: %.2f seconds\n", (secs/(t+1))*IN_COLS);
        printf("Remaining time: %.2f seconds\n\n", (secs/(t+1))*IN_COLS - secs);
    }

#endif

    printf("Done.");
    e_close(&dev);
    e_free(&mbuf);
    e_finalize();

    return EXIT_SUCCESS;
}
