#include <e-lib.h>
#include <stdlib.h>
#include "_e_lib_extended.h"
#include "common.h"

void sync_isr(int x);

int main(void) {
	unsigned *all_done_flag, *total_inf_clks, *total_up_clks, *section_clks, *slave_ready_flag, *slave_done_counter, *slave_inf_clks, *slave_up_clks, *masternode_clks, *p, slave_core_addr, t, j, k, all_ready, inf_clks, up_clks, timer_value_0, timer_value_1;
	float *xt, *dest;
	int *sample_no, batch_toggle;
	e_mutex_t *mutex;

	all_done_flag = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float));
    sample_no = (int *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + sizeof(unsigned));
    total_inf_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 2*sizeof(unsigned));
    total_up_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 3*sizeof(unsigned));
    masternode_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 4*sizeof(unsigned));
    section_clks = (unsigned *)(SHMEM_ADDR + IN_ROWS*IN_COLS*sizeof(float) + 5*sizeof(unsigned));

    slave_done_counter = (unsigned *)DONE_MEM_ADDR;
    mutex = (int *)DONE_MUTEX_MEM_ADDR;
    p = CLEAR_FLAG;
    batch_toggle = BATCH_TOGGLE;

    _e_global_mutex_init(MASTER_NODE_ROW, MASTER_NODE_COL, mutex, NULL);

    // Initialise benchmark results
    (*(all_done_flag)) = 0;
    (*(sample_no)) = -1;
    (*(total_inf_clks)) = 0;
    (*(total_up_clks)) = 0;
    (*(masternode_clks)) = 0;
    (*(section_clks)) = 0;

    e_irq_attach(E_SYNC, sync_isr);
    e_irq_mask(E_SYNC, E_FALSE);
    e_irq_global_mask(E_FALSE);

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

	for (t = 0; t < IN_COLS; t+=BATCH_STARTS) {
        e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
        e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

        if (IN_COLS - t == 1) batch_toggle = 0;

        for (j = NETWORK_ORIGIN_ROW; j < M + NETWORK_ORIGIN_ROW; ++j) {
            xt = (float *)(SHMEM_ADDR + (t+j*batch_toggle)*IN_ROWS*sizeof(float));

            for (k = NETWORK_ORIGIN_COL; k < N + NETWORK_ORIGIN_COL; ++k) {
                slave_core_addr = (unsigned)_e_get_global_address_on_chip(j, k, p);
                dest = (float *)(slave_core_addr + XT_MEM_ADDR);
                e_memcopy(dest, xt, WK_ROWS*sizeof(float));
            }
        }

        *slave_done_counter = 0;

        for (j = NETWORK_ORIGIN_ROW; j < M + NETWORK_ORIGIN_ROW; ++j) {
            for (k = NETWORK_ORIGIN_COL; k < N + NETWORK_ORIGIN_COL; ++k) {
                _e_global_address_irq_set(j, k, E_SYNC);
            }
        }

        e_ctimer_set(E_CTIMER_1, E_CTIMER_MAX);
        e_ctimer_start(E_CTIMER_1, E_CTIMER_CLK);

		// Put core in idle state while waiting for signal from network
        __asm__ __volatile__("idle");

        timer_value_1 = E_CTIMER_MAX - e_ctimer_stop(E_CTIMER_1);

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
        (*(sample_no)) = t;
        (*(total_inf_clks)) = inf_clks;
        (*(total_up_clks)) = up_clks;
        (*(masternode_clks)) = timer_value_0;
        (*(section_clks)) = timer_value_1;
	}

	// Raising "done" flag for host
    (*(all_done_flag)) = SET_FLAG;

	// Put core in idle state
    __asm__ __volatile__("idle");

    return EXIT_SUCCESS;
}

/*
* Function: sync_isr
* ------------------
* Override sync function
*
* x: arbitrary value
*
*/

inline void __attribute__((interrupt)) sync_isr(int x) {
    return;
}
