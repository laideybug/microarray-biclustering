#include <common.h>
#include <e-lib.h>

int main(void) {
	unsigned *dest, *xt, *done_0, *done_1, *done_2, slave_core, src_addr, xt_addr;
	int i, j, k, l, all_done, done[N];

	src_addr = (unsigned)XT_MEM_ADDR;

	done_0 = (unsigned *)DONE_MEM_ADDR_0;
	done[0] = done_0;
	done_1 = (unsigned *)DONE_MEM_ADDR_1;
	done[1] = done_1;
	done_2 = (unsigned *)DONE_MEM_ADDR_2;
	done[2] = done_2;

	for (i = 0; i < IN_COLS; ++i) {
		xt_addr = (unsigned)XT_SHMEM_ADDR + (i*IN_ROWS*sizeof(float));
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

            for (l = 0; l < N; ++l) {
                all_done += *done[l];
            }

            if (all_done == N) {
                break;
            }
        }
	}
}
