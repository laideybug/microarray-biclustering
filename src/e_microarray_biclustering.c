#include <e-lib.h>
#include <math.h>
#include <string.h>
#include "common.h"
#include "e_microarray_biclustering_utils.h"
#include "e_synch.h"

void adjustScaling(float scaling);
float sign(float value);
void sync_isr(int x);

int main(void) {
	unsigned  *done_flag, *inf_clks, *up_clks, *p, i, j, reps, slave_core_addr, out_mem_offset, timer_value_0, timer_value_1;
	float *xt, *wk, *update_wk, *nu_opt, *nu_k, *nu_k0, *nu_k1, *nu_k2, *dest, *scaling_incomplete, *scaling_incomplete_k, *rms_wk_incomplete, *rms_wk_incomplete_k, subgrad[WK_ROWS], scaling, rms_wk, rms_wk_reciprocol;
#ifdef USE_MASTER_NODE
    unsigned *ready_flag, master_node_addr, done_flag_counter;
    e_mutex_t *mutex;
#endif

	xt = (float *)XT_MEM_ADDR;	            // Address of xt (WK_ROWS x 1)
	wk = (float *)WK_MEM_ADDR;	            // Address of dictionary atom (WK_ROWS x 1)
	update_wk = (float *)UP_WK_MEM_ADDR;	// Address of update atom (WK_ROWS x 1)
	nu_opt = (float *)NU_OPT_MEM_ADDR;      // Address of optimal dual variable (WK_ROWS x 1)
	nu_k0 = (float *)NU_K0_MEM_ADDR;	    // Address of node 0 dual variable estimate (WK_ROWS x 1)
	nu_k1 = (float *)NU_K1_MEM_ADDR;        // Address of node 1 dual variable estimate (WK_ROWS x 1)
	nu_k2 = (float *)NU_K2_MEM_ADDR;        // Address of node 2 dual variable estimate (WK_ROWS x 1)

    p = CLEAR_FLAG;
    out_mem_offset = (unsigned)((e_group_config.core_row * e_group_config.group_cols + e_group_config.core_col)*sizeof(unsigned));

#ifdef USE_MASTER_NODE
    master_node_addr = (unsigned)e_get_global_address_on_chip(MASTER_NODE_ROW, MASTER_NODE_COL, p);
	ready_flag = (unsigned *)(master_node_addr + READY_MEM_ADDR + out_mem_offset);
	inf_clks = (unsigned *)(master_node_addr + INF_CLKS_MEM_ADDR + out_mem_offset);
    up_clks = (unsigned *)(master_node_addr + UP_CLKS_MEM_ADDR + out_mem_offset);
    done_flag = (unsigned *)(master_node_addr + DONE_MEM_ADDR);

    mutex = (int *)DONE_MUTEX_MEM_ADDR;
#else
	done_flag = (unsigned *)(SHMEM_ADDR + out_mem_offset);
    inf_clks = (unsigned *)(SHMEM_ADDR + (M_N*sizeof(unsigned)) + out_mem_offset);
    up_clks = (unsigned *)(SHMEM_ADDR + (2*M_N*sizeof(unsigned)) + out_mem_offset);
#endif

    // Address of this cores dual variable estimate
    nu_k = (float *)(NU_K0_MEM_ADDR + (e_group_config.core_col * NU_MEM_OFFSET));
    // Address of this cores incomplete scaling value
    scaling_incomplete = (float *)(INC_SCAL_MEM_ADDR + (e_group_config.core_row * sizeof(float)));
    // Address of this cores incomplete rms value
    rms_wk_incomplete = (float *)(INC_RMS_MEM_ADDR + (e_group_config.core_row * sizeof(float)));

    // Re-enable interrupts
    e_irq_attach(E_SYNC, sync_isr);
    e_irq_mask(E_SYNC, E_FALSE);
    e_irq_global_mask(E_FALSE);

    // Initialise barriers
    e_barrier_init(barriers, tgt_bars);

#ifdef USE_MASTER_NODE
    (*(ready_flag)) = SET_FLAG;
#endif

    while (1) {
        // Set timers for benchmarking
        e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
        e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

#ifdef USE_MASTER_NODE
        // Put core in idle state
        __asm__ __volatile__("idle");
#endif

		for (reps = 0; reps < NUM_ITER; ++reps) {
            *scaling_incomplete = 0.0f;
            scaling = 0.0f;

			for (i = 0; i < WK_ROWS; ++i) {
				/* subgrad = (nu-xt)*minus_mu_over_N */
				subgrad[i] = nu_opt[i] - xt[i];
				subgrad[i] = subgrad[i] * -MU_2 * ONE_OVER_N;
				/* scaling = (my_W_transpose*nu) */
				*scaling_incomplete = *scaling_incomplete + wk[i] * nu_opt[i];
			}

			// Exchange incomplete scaling values along column
            for (j = 0; j < e_group_config.group_rows; ++j) {
                if (j != e_group_config.core_row) {
                    slave_core_addr = (unsigned)e_get_global_address(j, e_group_config.core_col, p);
                    dest = (float *)(slave_core_addr + scaling_incomplete);
                    e_memcopy(dest, scaling_incomplete, WK_ROWS*sizeof(float));
                }
            }

            // Synch with all other cores
            e_barrier(barriers, tgt_bars);

            for (j = 0; j < e_group_config.group_rows; ++j) {
                scaling_incomplete_k = (float *)(INC_SCAL_MEM_ADDR + j*sizeof(float));
                scaling = scaling + *scaling_incomplete_k;
            }

			adjustScaling(scaling);

			for (i = 0; i < WK_ROWS; ++i) {
				/* D * diagmat(scaling*my_minus_mu) */
				nu_k[i] = wk[i] * scaling * -MU_2;
			}

	        // Exchange dual variable estimates along row
            for (j = 0; j < e_group_config.group_cols; ++j) {
                if (j != e_group_config.core_col) {
                    slave_core_addr = (unsigned)e_get_global_address(e_group_config.core_row, j, p);
                    dest = (float *)(slave_core_addr + nu_k);
                    e_memcopy(dest, nu_k, WK_ROWS*sizeof(float));
                }
            }

	    	// Synch with all other cores
            e_barrier(barriers, tgt_bars);

	    	// Average dual variable estimates
			for (i = 0; i < WK_ROWS; ++i) {
	            nu_opt[i] = nu_opt[i] + subgrad[i] + ((nu_k0[i] + nu_k1[i] + nu_k2[i]) * ONE_OVER_N);
			}
		}

		timer_value_0 = E_CTIMER_MAX - e_ctimer_stop(E_CTIMER_0);
		e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
        e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);

		*scaling_incomplete = 0.0f;
        scaling = 0.0f;

		for (i = 0; i < WK_ROWS; ++i) {
			/* scaling = (my_W_transpose*nu); */
			*scaling_incomplete = *scaling_incomplete + wk[i] * nu_opt[i];
		}

		// Exchange incomplete scaling values along column
        for (j = 0; j < e_group_config.group_rows; ++j) {
            if (j != e_group_config.core_row) {
                slave_core_addr = (unsigned)e_get_global_address(j, e_group_config.core_col, p);
                dest = (float *)(slave_core_addr + scaling_incomplete);
                e_memcopy(dest, scaling_incomplete, WK_ROWS*sizeof(float));
            }
        }

        // Synch with all other cores
        e_barrier(barriers, tgt_bars);

        for (j = 0; j < e_group_config.group_rows; ++j) {
            scaling_incomplete_k = (float *)(INC_SCAL_MEM_ADDR + j*sizeof(float));
            scaling = scaling + *scaling_incomplete_k;
        }

		adjustScaling(scaling);

		// Update dictionary atom
		*rms_wk_incomplete = 0.0f;
		rms_wk = 0.0f;

		// Create update atom (Y_opt)
		for (i = 0; i < WK_ROWS; ++i) {
			update_wk[i] =  MU_W * nu_opt[i] * scaling;
			wk[i] = wk[i] + update_wk[i];
			wk[i] = fmax(abs(wk[i])-BETA*MU_W, 0.0f) * sign(wk[i]);
	        *rms_wk_incomplete = *rms_wk_incomplete + wk[i] * wk[i];

	        // Resetting/initialising the dual variable and update atom
			update_wk[i] = 0.0f;
			nu_opt[i] = 0.0f;
		}

		// Exchange incomplete rms values along column
        for (j = 0; j < e_group_config.group_rows; ++j) {
            if (j != e_group_config.core_row) {
                slave_core_addr = (unsigned)e_get_global_address(j, e_group_config.core_col, p);
                dest = (float *)(slave_core_addr + rms_wk_incomplete);
                e_memcopy(dest, rms_wk_incomplete, WK_ROWS*sizeof(float));
            }
        }

		// Synch with all other cores
        e_barrier(barriers, tgt_bars);

        for (j = 0; j < e_group_config.group_rows; ++j) {
            rms_wk_incomplete_k = (float *)(INC_RMS_MEM_ADDR + j*sizeof(float));
            rms_wk = rms_wk + *rms_wk_incomplete_k;
        }

		rms_wk = sqrt(rms_wk);

		if (rms_wk > 1.0f) {
            rms_wk_reciprocol = 1.0f / rms_wk;

			for (i = 0; i < WK_ROWS; ++i) {
				wk[i] = wk[i] * rms_wk_reciprocol;
			}
		}

		timer_value_1 = E_CTIMER_MAX - e_ctimer_stop(E_CTIMER_0);

#ifdef USE_MASTER_NODE
        // Aqcuire the mutex lock
		e_global_mutex_lock(MASTER_NODE_ROW, MASTER_NODE_COL, mutex);
        // Write benchmark values
		(*(inf_clks)) = timer_value_0;
		(*(up_clks)) = timer_value_1;
		// Increment "done" flag for master node
		done_flag_counter = (*(done_flag));
		done_flag_counter = done_flag_counter + 1;
	   	(*(done_flag)) = done_flag_counter;

	   	// Release the mutex lock
	   	e_global_mutex_unlock(MASTER_NODE_ROW, MASTER_NODE_COL, mutex);

        // The last node to update sends an interrupt to master node
	   	if (done_flag_counter == M_N) {
            e_global_address_irq_set(MASTER_NODE_ROW, MASTER_NODE_COL, E_SYNC);
	   	}
#else
        // Write benchmark values
        (*(inf_clks)) = timer_value_0;
		(*(up_clks)) = timer_value_1;
		// Raising "done" flag
        (*(done_flag)) = SET_FLAG;
        // Put core in idle state
        __asm__ __volatile__("idle");

#endif
	}

    return EXIT_SUCCESS;
}

/*
* Function: adjustScaling
* -----------------------
* Adjusts the value of scaling
*
* scaling: the value to adjust
*
*/

inline void adjustScaling(float scaling) {
    if (scaling > GAMMA) {
        scaling = (scaling - GAMMA) * ONE_OVER_DELTA;
    } else if (scaling < -GAMMA) {
        scaling = (scaling + GAMMA) * ONE_OVER_DELTA;
    } else {
        scaling = 0.0f;
    }
}

/*
* Function: sign
* --------------
* Returns a float (-1.0, 0.0, 1.0) depending on
* the sign of value
*
* value: the value to check
*
*/

inline float sign(float value) {
	if (value > 0.0f) {
		return 1.0f;
	} else if (value < 0.0f) {
		return -1.0f;
	} else {
		return 0.0f;
	}
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
