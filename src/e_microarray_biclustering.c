#include <e-lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MU_W 0.005
#define GAMMA 0.5
#define DELTA 0.1
#define MU_2 0.01 
#define ALPHA 0
#define NUM_ITER 2000
#define ALPHA_W 0
#define BETA 0.01

int main(void) {
	unsigned *a, *b, *c, *d;
	int i;

	a = (unsigned *) 0x2000;	// Address of xt
	b = (unsigned *) 0x4000;	// Address of dictionary atom
	c = (unsigned *) 0x6000;	// Address of update atom
	d = (unsigned *) 0x7000;	// "Done" flag

	e_coreid_t id = e_get_coreid();
	unsigned row, col;

	// Getting the row, column coordinates of this core
	e_coords_from_coreid(id, &row, &col);

	for (i = 0; i < NUM_ITER; ++i) {
		
	}

	// Raising "done" flag
   	(*(d)) = 0x00000001;
 
   	// Put core in idle state
   	__asm__ __volatile__("idle");
}
