#include <common.h>
#include <e-lib.h>
#include <math.h>
#include <static_buffers.h>
#include <stdlib.h>

#define NU_MEM_OFFSET 0x0300

void adjustScaling(float scaling);
int sign(float value);

int main(void) {
	unsigned *xt, *wk, *update_wk, *nu_opt, *nu_k0, *nu_k1, *nu_k2, *done_flag, *src, *dest, *p;
	unsigned slave_core, j, k, dest_addr;
	int i, reps;
	float subgrad[IN_ROWS];
	float scaling;

	xt = (unsigned *) XT_MEM_ADDR;	        // Address of xt (56 x 1)
	wk = (unsigned *) WK_MEM_ADDR;	        // Address of dictionary atom (56 x 1)
	update_wk = (unsigned *) UP_WK_MEM_ADDR;	// Address of update atom (56 x 1)
	nu_opt = (unsigned *) NU_OPT_MEM_ADDR;       // Address of optimal dual variable (56 x 1)
	nu_k0 = (unsigned *) NU_K0_MEM_ADDR;	    // Address of this cores dual variable estimate (56 x 1)
	nu_k1 = (unsigned *) NU_K1_MEM_ADDR;        // Address of neighbour 1 dual variable estimate (56 x 1)
	nu_k2 = (unsigned *) NU_K2_MEM_ADDR;        // Address of neighbour 2 dual variable estimate (56 x 1)
	done_flag = (unsigned *) DONE_MEM_ADDR;	// "Done" flag (1 x 1)
    p = 0x0000;

    // Initialise barriers
    e_barrier_init(barriers, tgt_bars);

    while (1) {
		// Resetting/Initialising the dual variable and update atom
		for (i = 0; i < IN_ROWS; ++i) {
			nu_opt[i] = 0.0f;
			update_wk[i] = 0.0f;
		}

		scaling = 0.0f;

		for (reps = 0; reps < NUM_ITER; ++reps) {
			for (i = 0; i < IN_ROWS; ++i) {
				/* subgrad = (nu-xt)*minus_mu_over_N */
				subgrad[i] = nu_opt[i] - xt[i];
				subgrad[i] *= (-MU_2 / N);
			}

			for (i = 0; i < IN_ROWS; ++i) {
				/* scaling = (my_W_transpose*nu) */
				scaling += wk[i] * nu_opt[i];
			}

			adjustScaling(scaling);

			for (i = 0; i < IN_ROWS; ++i) {
				/* D * diagmat(scaling*my_minus_mu) */
				nu_k0[i] = wk[i] * (scaling * -MU_2);
			}

			// Synch with all other cores
			e_barrier(barriers, tgt_bars);

			dest_addr = (unsigned)NU_K0_MEM_ADDR;

	        // Exchange dual variable estimates
			for (j = 0; j < e_group_config.group_rows; ++j) {
	            for (k = 0; k < e_group_config.group_cols; ++k) {
	                if ((j != e_group_config.core_row) | (k != e_group_config.core_col)) {
	                    slave_core = (unsigned)e_get_global_address(j, k, p);
	                    src = (unsigned *)(slave_core + (unsigned)NU_K0_MEM_ADDR);
	                    dest_addr = dest_addr + (unsigned)NU_MEM_OFFSET;
	                    dest = (unsigned *)(dest_addr);
	                    e_dma_copy(dest, src, IN_ROWS*sizeof(float));
	                }
	            }
			}

	    	// Synch with all other cores
	    	e_barrier(barriers, tgt_bars);

	    	// Average dual variable estimates
			for (i = 0; i < IN_ROWS; ++i) {
	            float mean_nu_i = (nu_k0[i] + nu_k1[i] + nu_k2[i]) / N;
	            nu_opt[i] = nu_opt[i] + subgrad[i] + mean_nu_i;
			}
		}

		for (i = 0; i < IN_ROWS; ++i) {
			/* scaling = (my_W_transpose*nu); */
			scaling += wk[i] * nu_opt[i];
		}

		adjustScaling(scaling);

		// Create update atom (Y_opt)
		for (i = 0; i < IN_ROWS; ++i) {
			update_wk[i] =  MU_W * (nu_opt[i] * scaling);
		}

		// Update dictionary atom
		float rms_wk = 0.0f;

		for (i = 0; i < IN_ROWS; ++i) {
			wk[i] += update_wk[i];
			wk[i] = fmax(abs(wk[i])-BETA*MU_W, 0.0f) * sign(wk[i]);

	        rms_wk += pow(wk[i], 2);
		}

		rms_wk = sqrt(rms_wk);

		if (rms_wk > 1.0f) {
			for (i = 0; i < IN_ROWS; ++i) {
				wk[i] /= rms_wk;
			}
		}

		// Raising "done" flag
	   	(*(done_flag)) = 1;

	   	// Put core in idle state
   		__asm__ __volatile__("idle");
	}
}

/*
* Function: adjustScaling
* -----------------------
* Adjusts the value of scaling
* depending on it's relation to
* the value of GAMMA
*
* scaling: the value to adjust
*/

void adjustScaling(float scaling) {
	if (scaling > GAMMA) {
		scaling = (scaling - GAMMA) / DELTA;
	} else if (scaling < -GAMMA) {
		scaling = (scaling + GAMMA) / DELTA;
	} else {
		scaling = 0;
	}
}

/*
* Function: sign
* --------------
* Returns an integer (-1, 0, 1) depending on
* the sign of value
*
* value: the value to check
*/

int sign(float value) {
	if (value > 0.0f) {
		return 1;
	} else if (value < 0.0f) {
		return -1;
	} else {
		return 0;
	}
}
