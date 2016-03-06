#include <e-lib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IN_ROWS 56
#define IN_COLS 12625
#define N 3
#define MU_W 0.005
#define GAMMA 0.5
#define DELTA 0.1
#define MU_2 0.01
#define ALPHA 0
#define NUM_ITER 2000
#define ALPHA_W 0
#define BETA 0.01

void adjustScaling(float scaling);
void mean();

int main(void) {
	unsigned *xt, *wk, *up_k, *nu_k, *done_flag;
	int i, reps;
	float nu[IN_ROWS], subgrad[IN_ROWS];
	float scaling = 0.0f;

	xt = (unsigned *) 0x2000;	// Address of xt (56 x 1)
	wk = (unsigned *) 0x3000;	// Address of dictionary atom
	up_k = (unsigned *) 0x4000;	// Address of update atom
	nu_k = (unsigned *) 0x5000;	// Address of dual variable
	done_flag = (unsigned *) 0x7000;	// "Done" flag

	e_coreid_t id = e_get_coreid();
	unsigned row, col;

	// Getting the row, column coordinates of this core
	e_coords_from_coreid(id, &row, &col);

	// Initialising the "dual variable"
	for (i = 0; i < IN_ROWS; ++i) {
		nu[i] = 0;
	}

	for (reps = 0; reps < NUM_ITER; ++reps) {
		for (i = 0; i < IN_ROWS; ++i) {
			/* subgrad = (nu-xt)*minus_mu_over_N */
			subgrad[i] = nu[i] - xt[i];
			subgrad[i] *= (-MU_2 / N);
		}

		for (i = 0; i < IN_ROWS; ++i) {
			/* scaling = (my_W_transpose*nu) */
			scaling += wk[i] * nu[i];
		}

		adjustScaling(scaling);

		for (i = 0; i < IN_ROWS; ++i) {
			/* D * diagmat(scaling*my_minus_mu) */
			nu_k[i] = wk[i] * (scaling * -MU_2);
		}

		// SYNCH

		// e_write(&dev, 0, 0, 0x6000, &sync_data, sizeof(sync_data));

    	// SYNCH

		for (i = 0; i < IN_ROWS; ++i) {
			// nu[i] = nu[i] + subgrad[i] +
		}
	}

	for (i = 0; i < IN_ROWS; ++i) {
		/* scaling = (my_W_transpose*nu); */
		scaling += wk[i] * nu[i];
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

/*
* Function: mean
* --------------
*
*/

void mean() {
	// float temp[IN_ROWS];
	// float column_total;

	// for (j = 0; j < cols; ++j) {
 //    	column_total = 0.0f;

 //    	for (k = 0; k < rows; ++k) {
 //        	column_total += matrix[k][j];
 //    	}

 //        mean_vector[j] = column_total / rows;
	// }
}
