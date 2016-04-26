#include <e-lib.h>
#include <stdlib.h>
#include "_e_lib_extended.h"
#include "common.h"

int main(void) {
	unsigned *all_done_flag, *total_inf_clks, *total_up_clks, *masternode_clks, *p, slave_core_addr, t, j, k, all_ready, inf_clks, up_clks, timer_value_0;
	volatile unsigned *slave_ready_flag, *slave_inf_clks, *slave_up_clks, *slave_done_counter, slave_done_count;
	float *xt, *next_sample, *dest;
	volatile float *slave_scaling_vals, *scaling_vals;
	int *sample_no;
	e_mutex_t *mutex;

	all_done_flag = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float));
    sample_no = (int *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + sizeof(unsigned));
    total_inf_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 2*sizeof(unsigned));
    total_up_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 3*sizeof(unsigned));
    masternode_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 4*sizeof(unsigned));
    scaling_vals = (float *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 5*sizeof(unsigned));

    next_sample = (float *)SHMEM_ADDR;
    slave_done_counter = (unsigned *)DONE_MEM_ADDR;
    slave_scaling_vals = (float *)SCAL_MEM_ADDR;
    mutex = (int *)DONE_MUTEX_MEM_ADDR;
    p = CLEAR_FLAG;

    _e_global_mutex_init(MASTER_NODE_ROW, MASTER_NODE_COL, mutex, NULL);

    // Copy first sample to node memory
    for (j = 0; j < M; ++j) {
        xt = (float *)(MASTER_XT_MEM_ADDR + j*WK_ROWS*sizeof(float));
        e_memcopy(xt, next_sample, WK_ROWS*sizeof(float));
    }

    // Initialise benchmark results
    (*(all_done_flag)) = 0;
    (*(total_inf_clks)) = 0;
    (*(total_up_clks)) = 0;
    (*(masternode_clks)) = 0;
    (*(sample_no)) = 0;

    while (1) {
        all_ready = 0;

        for (j = 0; j < M_N; ++j) {
            slave_ready_flag = (unsigned *)(READY_MEM_ADDR + j*sizeof(unsigned));
            all_ready += *slave_ready_flag;
        }

        if (all_ready == M_N) {
            break;
        }
    }

	for (t = 1; t <= IN_COLS; t+=BATCH_STARTS) {
        e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
        e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

        slave_done_count = 0;
        *slave_done_counter = 0;

        for (j = NETWORK_ORIGIN_ROW; j < M + NETWORK_ORIGIN_ROW; ++j) {
            xt = (float *)(MASTER_XT_MEM_ADDR + j*WK_ROWS*sizeof(float));

            for (k = NETWORK_ORIGIN_COL; k < N + NETWORK_ORIGIN_COL; ++k) {
                slave_core_addr = (unsigned)_e_get_global_address_on_chip(j, k, p);
                dest = (float *)(slave_core_addr + XT_MEM_ADDR);
                e_memcopy(dest, xt, WK_ROWS*sizeof(float));
                _e_global_address_irq_set(j, k, E_SYNC);
            }
        }

        if (IN_COLS - t != 0) {
            for (j = 0; j < M; ++j) {
                next_sample = (float *)(SHMEM_ADDR + (t+j)*IN_ROWS*sizeof(float));
                xt = (float *)(MASTER_XT_MEM_ADDR + j*WK_ROWS*sizeof(float));
                e_memcopy(xt, next_sample, WK_ROWS*sizeof(float));
            }
        }

		// Wait for all slaves to finish
        while (slave_done_count != M_N) {
            _e_global_mutex_lock(MASTER_NODE_ROW, MASTER_NODE_COL, mutex);
            slave_done_count = *slave_done_counter;
            _e_global_mutex_unlock(MASTER_NODE_ROW, MASTER_NODE_COL, mutex);
        }

        inf_clks = 0;
        up_clks = 0;

        for (j = 0; j < M_N; ++j) {
            slave_inf_clks = (unsigned *)(INF_CLKS_MEM_ADDR + j*sizeof(unsigned));
            inf_clks += *slave_inf_clks;
            *slave_inf_clks = 0;

            slave_up_clks = (unsigned *)(UP_CLKS_MEM_ADDR + j*sizeof(unsigned));
            up_clks += *slave_up_clks;
            *slave_up_clks = 0;
        }

        timer_value_0 = E_CTIMER_MAX - e_ctimer_stop(E_CTIMER_0);

        // Write benchmark results
        e_memcopy(scaling_vals, slave_scaling_vals, M_N*sizeof(float));
        (*(total_inf_clks)) = inf_clks;
        (*(total_up_clks)) = up_clks;
        (*(masternode_clks)) = timer_value_0;
        (*(sample_no)) = t;
	}

	// Raising "done" flag for host
    (*(all_done_flag)) = SET_FLAG;

	// Put core in idle state
    __asm__ __volatile__("idle");

    return EXIT_SUCCESS;
}
