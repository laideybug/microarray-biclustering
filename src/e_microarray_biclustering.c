#include <common.h>
#include <e-lib.h>
#include <math.h>
#include <static_buffers.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NU_MEM_OFFSET 0x0300

void adjustScaling(float scaling);

int main(void) {
	unsigned *xt, *wk, *up_k, *nu_opt, *nu_k0, *nu_k1, *nu_k2, *done_flag, *src, *dest, *p;
	unsigned slave_core;
	int i, j, k, reps;
	float subgrad[IN_ROWS];
	float scaling = 0.0f;

	xt = (unsigned *) 0x2000;	        // Address of xt (56 x 1)
	wk = (unsigned *) 0x3000;	        // Address of dictionary atom
	up_k = (unsigned *) 0x4000;	        // Address of update atom
	nu_opt = (unsigned *) 0x4600;       // Address of optimal dual variable
	nu_k0 = (unsigned *) 0x5000;	    // Address of this cores dual variable estimate
	nu_k1 = (unsigned *) 0x5300;        // Address of neighbour 1 dual variable estimate
	nu_k2 = (unsigned *) 0x5600;        // Address of neighbour 2 dual variable estimate
	done_flag = (unsigned *) 0x7000;	// "Done" flag

    // Initialise barriers
    e_barrier_init(barriers, tgt_bars);

	// Initialising the "dual variable"
	for (i = 0; i < IN_ROWS; ++i) {
		nu_opt[i] = 0.0f;
	}

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

		dest = nu_k0;

        // Exchange dual variable estimates
		for (j = 0; j < e_group_config.group_rows; ++j) {
            for (k = 0; k < e_group_config.group_cols; ++k) {
                if ((j != e_group_config.core_row) | (k != e_group_config.core_col)) {
                    slave_core = (unsigned)e_get_global_address(i, j, p);
                    src = slave_core + nu_k0;
                    dest = dest + NU_MEM_OFFSET;
                    e_dma_copy(nu_k0 + NU_MEM_OFFSET, src, IN_ROWS*sizeof(float));
                }
            }
		}

    	// Synch with all other cores
    	e_barrier(barriers, tgt_bars);

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

	// Raising "done" flag
   	(*(done_flag)) = 0x00000001;

   	// Put core in idle state
   	__asm__ __volatile__("idle");
}

/*
* Function: adjustScaling
* -----------------------
*
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
