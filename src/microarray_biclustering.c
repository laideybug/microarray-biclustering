#include <e-hal.h>
#include <e-loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "mb_utils.h"

#define SHM_OFFSET 0x01000000

typedef struct fl_ind {
    float value;
    unsigned index;
} fl_ind;

int cmp(const void *a, const void *b);

int main(int argc, char *argv[]) {
    unsigned current_row, current_col, i, j, k, all_done, avg_inf_clks, avg_up_clks, total_inf_clks, total_up_clks, clr;
    float xt[IN_ROWS], input_data[IN_ROWS][IN_COLS], dictionary_w[IN_ROWS][N], dictionary_wk[IN_ROWS], dictionary_wk_i[WK_ROWS], output_dict[IN_ROWS][N], update_wk[WK_ROWS], dual_var[WK_ROWS], scaling_matrix[N][IN_COLS], scaling_k[IN_COLS], scaling_vals[BATCH_STARTS_N], data_point, secs, t_plus_one_reciprocol;
    int t, batch_starts;
    struct fl_ind norms[N];
    FILE *input_file, *output_file;
#ifdef USE_MASTER_NODE
    unsigned masternode_clks, section_clks;
    int previous_t;
#else
    unsigned done[M_N], inf_clks, up_clks;
    float xt_k[WK_ROWS];
    int batch_toggle;
#endif

    // Seed the random number generator
    srand(1);
    clr = CLEAR_FLAG;

    printf("\nReading input data...\n");

    // Read input data
    input_file = fopen(DATA_PATH, "r");

    if (input_file != NULL) {
        current_row = 0;
        current_col = 0;

        while (fscanf(input_file, "%f", &data_point) != EOF) {
            input_data[current_row][current_col] = data_point;

            // data.txt is in 12625 x 56 format -
            // we need to model it as a
            // 56 x 12625 matrix
            if (current_row < (IN_ROWS - 1)) {
                current_row++;
            } else {
                current_row = 0;
                current_col++;
            }
        }

        fclose(input_file);
    } else {
        printf("Error: Failed to open input file.\n");
        return EXIT_FAILURE;
    }

    printf("Initialising dictionary...\n");

    // Remove DC component from input data (i.e. centering)
    mb_remove_dc(IN_ROWS, IN_COLS, input_data);

    // Randomly initialise and normalise the dictionary
    mb_fill_matrix_random(IN_ROWS, N, dictionary_w);
    mb_scalar_multiply(IN_ROWS, N, dictionary_w, 10.0f);
    mb_norm_matrix(IN_ROWS, N, dictionary_w);

    for (j = 0; j < IN_ROWS; ++j) {
        for (k = 0; k < N; ++k) {
            printf("%f ", dictionary_w[j][k]);
        }

        printf("\n");
    }

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
    e_open(&dev, NETWORK_ORIGIN_ROW, NETWORK_ORIGIN_COL, M, N);
    e_reset_group(&dev);

    // Initialise update dictionary and dual variable vectors with 0
    mb_fill_vector(WK_ROWS, update_wk, 0.0f);
    mb_fill_vector(WK_ROWS, dual_var, 0.0f);
    mb_fill_vector(IN_ROWS, scaling_k, 0.0f);
    mb_fill_matrix(N, IN_COLS, scaling_matrix, 0.0f);

    // Load the dictionary atoms into each core
    for (j = 0; j < M; ++j) {
        for (k = 0; k < N; ++k) {
            mb_get_matrix_column(IN_ROWS, N, k, dictionary_w, dictionary_wk);

            for (i = 0; i < WK_ROWS; ++i) {
#ifdef BATCH_DISTRIBUTED
                dictionary_wk_i[i] = *(dictionary_wk + i);
#else
                dictionary_wk_i[i] = *(dictionary_wk + (i + j*WK_ROWS));
#endif
            }

            e_write(&dev, j, k, WK_MEM_ADDR, &dictionary_wk_i, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, UP_WK_MEM_ADDR, &update_wk, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_K0_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_K1_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_K2_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K0_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K1_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K2_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
            e_write(&dev, j, k, NU_OPT_K3_MEM_ADDR, &dual_var, WK_ROWS*sizeof(float));
        }
    }

     // Load program to the workgroup but do not run yet
    if (e_load_group(AGENT_BIN_PATH, &dev, 0, 0, M, N, E_FALSE) != E_OK) {
        printf("Error: Failed to load agent program\n");
        return EXIT_FAILURE;
    }

#ifdef USE_MASTER_NODE
    // Open the master node workgroup
    e_epiphany_t dev_master;
    e_open(&dev_master, MASTER_NODE_ROW, MASTER_NODE_COL, 1, 1);
    e_reset_group(&dev_master);

    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, IN_ROWS*IN_COLS*sizeof(float) + MASTER_BENCH_NUM*sizeof(unsigned)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    // Write input data to shared memory
    for (i = 0; i < IN_COLS; ++i) {
        mb_get_matrix_column(IN_ROWS, IN_COLS, i, input_data, xt);
        e_write(&mbuf, 0, 0, i*IN_ROWS*sizeof(float), &xt, IN_ROWS*sizeof(float));
    }

    // Clear done flag in shared memory
    e_write(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float), &clr, sizeof(unsigned));

    // Load program to the master core but do not run yet
    if (e_load(MASTER_BIN_PATH, &dev_master, 0, 0, E_FALSE) != E_OK) {
        printf("Error: Failed to load master program\n");
        return EXIT_FAILURE;
    }

    previous_t = -1;
    secs = 0.0f;
    batch_starts = BATCH_STARTS;

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
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float) + (5*sizeof(unsigned)), &section_clks, sizeof(unsigned));
        e_read(&mbuf, 0, 0, IN_ROWS*IN_COLS*sizeof(float) + (6*sizeof(unsigned)), &scaling_vals, BATCH_STARTS_N*sizeof(float));

        if (t - previous_t) {
            avg_inf_clks = (unsigned)(total_inf_clks * ONE_OVER_M_N);
            avg_up_clks = (unsigned)(total_up_clks * ONE_OVER_M_N);
            previous_t = t;
            secs += masternode_clks * ONE_OVER_E_CYCLES;
            t_plus_one_reciprocol = 1.0f/(t+1);

#ifdef BATCH_DISTRIBUTED
            if (IN_COLS - t == 1) batch_starts = 1;
#endif
            // Collect scaling values
            for (j = 0; j < batch_starts; ++j) {
                for (k = 0; k < N; ++k) {
                    scaling_matrix[k][t + j] = scaling_vals[j*N + k];
                }
            }

            printf("\nMode: %s\n", MODE);
            printf("Config: Master Node, %i x %i\n", M, N);
            printf("---------------------------------------\n");
#ifdef BATCH_DISTRIBUTED
            printf("Processed input samples: %u - %u\n", t+1, t+batch_starts);
#else
            printf("Processed input sample: %u\n", t+1);
#endif
            printf("Average clock cycles for inference step: %u clock cycles\n", avg_inf_clks);
            printf("Average network speed of inference step: %.6f seconds\n", avg_inf_clks * ONE_OVER_E_CYCLES);
            printf("Average clock cycles for update step: %u clock cycles\n", avg_up_clks);
            printf("Average network speed of update step: %.6f seconds\n", avg_up_clks * ONE_OVER_E_CYCLES);
            printf("Master node clock cycles: %u clock cycles\n", masternode_clks);
            printf("Section clock cycles: %u clock cycles\n", section_clks);
            printf("-------------------------------\n");
            printf("Percent complete: %.2f%%\n", (t+1)*100.0f*ONE_OVER_IN_COLS);
            printf("Average speed: %.6f seconds/sample\n", secs*t_plus_one_reciprocol);
            printf("Time elapsed: %.2f seconds\n", secs);
            printf("Total time estimate: %.2f seconds\n", secs*t_plus_one_reciprocol*IN_COLS);
            printf("Remaining time estimate: %.2f seconds\n\n", secs*t_plus_one_reciprocol*IN_COLS - secs);
        }

        if (all_done == 1) {
            break;
        }
    }

    e_close(&dev_master);

#else
    // Allocate shared memory
    if (e_alloc(&mbuf, SHM_OFFSET, 3*M_N*sizeof(unsigned) + BATCH_STARTS_N*sizeof(float)) != E_OK) {
        printf("Error: Failed to allocate shared memory\n");
        return EXIT_FAILURE;
    };

    printf("Network started...\n\n");

    struct timeval start, end;
    gettimeofday(&start, NULL);
    secs = 0.0f;
    batch_toggle = BATCH_TOGGLE;
    batch_starts = BATCH_STARTS;

    for (t = 0; t < IN_COLS; t+=BATCH_STARTS) {
#ifdef BATCH_DISTRIBUTED
        if (IN_COLS - t == 1) {
            batch_toggle = 0;
            batch_starts = 1;
        }
#endif

        for (j = 0; j < M; ++j) {
            mb_get_matrix_column(IN_ROWS, IN_COLS, t+j*batch_toggle, input_data, xt);

            for (k = 0; k < WK_ROWS; ++k) {
                xt_k[k] = *(xt + (k + j*WK_ROWS));
            }

            for (k = 0; k < N; ++k) {
                // "Stream" next data sample
                e_write(&dev, j, k, XT_MEM_ADDR, &xt_k, WK_ROWS*sizeof(float));
                // Clear done flag
                e_write(&mbuf, 0, 0, (j*N+k)*sizeof(unsigned), &clr, sizeof(unsigned));
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

        e_read(&mbuf, 0, 0, (3*M_N*sizeof(unsigned)), &scaling_vals, BATCH_STARTS_N*sizeof(float));

        // Collect scaling values
        for (j = 0; j < batch_starts; ++j) {
            for (k = 0; k < N; ++k) {
                scaling_matrix[k][t + j] = scaling_vals[j*N + k];
            }
        }

        avg_inf_clks = (unsigned)(total_inf_clks * ONE_OVER_M_N);
        avg_up_clks = (unsigned)(total_up_clks * ONE_OVER_M_N);

        gettimeofday(&end, NULL);
        secs = ((end.tv_sec + end.tv_usec * 0.000001) - (start.tv_sec + start.tv_usec * 0.000001));
        t_plus_one_reciprocol = 1.0f/(t+1);

        printf("\nMode: %s\n", MODE);
        printf("Config: Master Node, %i x %i\n", M, N);
        printf("-------------------------------\n");
#ifdef BATCH_DISTRIBUTED
        printf("Processed input samples: %u - %u\n", t+1, t+batch_starts);
#else
        printf("Processed input sample: %u\n", t+1);
#endif
        printf("Average clock cycles for inference step: %u clock cycles\n", avg_inf_clks);
        printf("Average network speed of inference step: %.6f seconds\n", avg_inf_clks * ONE_OVER_E_CYCLES);
        printf("Average clock cycles for update step: %u clock cycles\n", avg_up_clks);
        printf("Average network speed of update step: %.6f seconds\n", avg_up_clks * ONE_OVER_E_CYCLES);
        printf("-------------------------------\n");
        printf("Percent complete: %.2f%%\n", (t+1)*100.0f*ONE_OVER_IN_COLS);
        printf("Average speed: %.6f seconds/sample\n", secs*t_plus_one_reciprocol);
        printf("Time elapsed: %.2f seconds\n", secs);
        printf("Total time: %.2f seconds\n", secs*t_plus_one_reciprocol*IN_COLS);
        printf("Remaining time: %.2f seconds\n\n", secs*t_plus_one_reciprocol*IN_COLS - secs);
    }

#endif

    printf("Done.\n");
    printf("Gathering learned dictionary...\n");

    // Get learned dictionary
#ifdef BATCH_DISTRIBUTED
    for (k = 0; k < N; ++ k) {
        e_read(&dev, 0, k, WK_MEM_ADDR, &dictionary_wk_i, WK_ROWS*sizeof(float));

        for (i = 0; i < WK_ROWS; ++i) {
            dictionary_w[i][k] = dictionary_wk_i[i];
        }
    }
#else
    for (j = 0; j < M; ++j) {
        for (k = 0; k < N; ++ k) {
            e_read(&dev, j, k, WK_MEM_ADDR, &dictionary_wk_i, WK_ROWS*sizeof(float));

            for (i = 0; i < WK_ROWS; ++i) {
                dictionary_w[i + j*WK_ROWS][k] = dictionary_wk_i[i];
            }
        }
    }
#endif

    for (i = 0; i < N; ++i) {
        mb_get_matrix_row(N, IN_COLS, i, scaling_matrix, scaling_k);
        norms[i].value = mb_norm_vector(IN_COLS, scaling_k);
        norms[i].index = i;
    }

    qsort(norms, N, sizeof(norms[0]), cmp);

    for (j = 0; j < IN_ROWS; ++j) {
        for (k = 0; k < N; ++k) {
            output_dict[j][k] = dictionary_w[j][norms[k].index];
            printf("%f ", dictionary_w[j][k]);
        }

        printf("\n");
    }

    for (j = 0; j < IN_ROWS; ++j) {
        for (k = 0; k < N; ++k) {
            printf("%f ", dictionary_w[j][k]);
        }

        printf("\n");
    }

    // Output data to .dat file here
    output_file = fopen(OUT_PATH, "wb+");

    if (output_file != NULL) {
        for (j = 0; j < IN_ROWS; ++j) {
            for (k = 0; k < N; ++k) {
                fprintf(output_file, "%f ", output_dict[j][k]);
            }

            fprintf(output_file, "\n");
        }

        fclose(output_file);
    } else {
        printf("Error: Failed to create output file.\n");
        return EXIT_FAILURE;
    }

    printf("\nDone. Plot results with './plot.sh'\n\n");

    e_close(&dev);
    e_free(&mbuf);
    e_finalize();

    return EXIT_SUCCESS;
}

/*
* Function: cmp
* -------------
* Comparator function to be used with qsort
* to obtain a DESCENDING value sort
*
* a: first value to compare
* b: second value to compare
*
*/

int cmp(const void *a, const void *b) {
    struct fl_ind *a_1 = (struct fl_ind *)a;
    struct fl_ind *a_2 = (struct fl_ind *)b;

    if ((*a_1).value > (*a_2).value) {
        return -1;
    } else if ((*a_1).value < (*a_2).value) {
        return 1;
    }

    return 0;
}

