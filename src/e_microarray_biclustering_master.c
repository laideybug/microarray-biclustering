#include <e-lib.h>
#include "common.h"

int main(void) {
	unsigned *dest, *xt, *done_0, *done_1, *done_2, *done_flag, *p;
	unsigned done_flag_addr, slave_core, src_addr, xt_addr;
	int i, j, k, l, all_done, done[N];

	src_addr = (unsigned)XT_MEM_ADDR;

	done_0 = (unsigned *)DONE_MEM_ADDR_0;
	done[0] = *done_0;
	done_1 = (unsigned *)DONE_MEM_ADDR_1;
	done[1] = *done_1;
	done_2 = (unsigned *)DONE_MEM_ADDR_2;
	done[2] = *done_2;

    p = 0x0000;
	done_flag_addr = (unsigned)SHMEM_ADDR + (IN_ROWS*IN_COLS*sizeof(float));
	done_flag = (unsigned *)done_flag_addr;

	for (i = 0; i < IN_COLS; ++i) {
		xt_addr = (unsigned)SHMEM_ADDR + (i*IN_ROWS*sizeof(float));
		xt = (unsigned *)xt_addr;

		for (j = 0; j < e_group_config.group_rows; ++j) {
	        for (k = 0; k < e_group_config.group_cols; ++k) {
	            if ((j != e_group_config.core_row) | (k != e_group_config.core_col)) {
                    slave_core = (unsigned)e_get_global_address(j, k, p);
	                dest = (unsigned *)(slave_core + (unsigned)src_addr);
	                e_dma_copy(dest, xt, IN_ROWS*sizeof(float));
	                e_irq_set(j, k, E_SYNC);
	            }
	        }
		}

		while (1) {
            all_done = 0;

            for (j = 0; j < N; ++j) {
                all_done += done[l];
            }

            if (all_done == N) {
                done[0] = 0;
                done[1] = 0;
                done[2] = 0;
                break;
            }
        }
	}

	// Raising "done" flag for host
    (*(done_flag)) = 0x00000001;

	// Put core in idle state
    __asm__ __volatile__("idle");

    return EXIT_SUCCESS;
}
