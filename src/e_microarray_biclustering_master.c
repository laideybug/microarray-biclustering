#include <e-lib.h>
#include "common.h"
#include "e_microarray_biclustering_utils.h"

int main(void) {
	unsigned *all_done_flag, *total_inf_clks, *total_up_clks, *slave_ready_flag, *slave_done_flag, *slave_inf_clks, *slave_up_clks, *masternode_clks, *p, slave_core_addr, i, j, k, all_ready, all_done, inf_clks, up_clks;
	float *xt, *dest, xt_k[WK_ROWS];
	int *sample_no;

	all_done_flag = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float));
    sample_no = (int *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + sizeof(unsigned));
    total_inf_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 2*sizeof(unsigned));
    total_up_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 3*sizeof(unsigned));
    masternode_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 4*sizeof(unsigned));

    p = CLEAR_FLAG;

    // Initialise benchmark results
    (*(sample_no)) = -1;
    (*(total_inf_clks)) = 0;
    (*(total_up_clks)) = 0;
    (*(masternode_clks)) = 0;
    (*(all_done_flag)) = 0;

    while (1) {
        all_ready = 0;

        for (i = 0; i < M*N; ++i) {
            slave_ready_flag = (unsigned *)(READY_MEM_ADDR + i*sizeof(unsigned));
            all_ready += *slave_ready_flag;
        }

        if (all_ready == M*N) {
            break;
        }
    }

	for (i = 0; i < IN_COLS; ++i) {
        e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
        e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

		xt = (float *)(SHMEM_ADDR + i*IN_ROWS*sizeof(float));

        for (j = 0; j < M; ++j) {
            for (k = 0; k < WK_ROWS; ++k) {
                xt_k[k] = *(xt + (k + j*WK_ROWS));
            }

            for (k = 0; k < N; ++k) {
                slave_core_addr = (unsigned)e_get_global_address_on_chip(j, k, p);
                dest = (float *)(slave_core_addr + XT_MEM_ADDR);
                e_memcopy(dest, xt_k, WK_ROWS*sizeof(float));
                e_global_address_irq_set(j, k, E_SYNC);
            }
        }

		while (1) {
            all_done = 0;

            for (j = 0; j < M*N; ++j) {
                slave_done_flag = (unsigned *)(DONE_MEM_ADDR + j*sizeof(unsigned));
                all_done += *slave_done_flag;
            }

            if (all_done == M*N) {
                break;
            }
        }

        inf_clks = 0;
        up_clks = 0;

        for (j = 0; j < M*N; ++j) {
            slave_inf_clks = (unsigned *)(INF_CLKS_MEM_ADDR + j*sizeof(unsigned));
            inf_clks += *slave_inf_clks;
            *slave_inf_clks = 0;

            slave_up_clks = (unsigned *)(UP_CLKS_MEM_ADDR + j*sizeof(unsigned));
            up_clks += *slave_up_clks;
            *slave_up_clks = 0;

            slave_done_flag = (unsigned *)(DONE_MEM_ADDR + j*sizeof(unsigned));
            *slave_done_flag = 0;
        }

        // Write benchmark results
        (*(sample_no)) = i;
        (*(total_inf_clks)) = inf_clks;
        (*(total_up_clks)) = up_clks;
        (*(masternode_clks)) = E_CTIMER_MAX - e_ctimer_stop(E_CTIMER_0);
	}

	// Raising "done" flag for host
    (*(all_done_flag)) = SET_FLAG;

	// Put core in idle state
    __asm__ __volatile__("idle");

    return EXIT_SUCCESS;
}
