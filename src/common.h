#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define IN_ROWS			    56
#define IN_COLS			    12625
#define N 				    3
#define ONE_OVER_N 		    0.333333333
#define PI 				    3.141592654
#define MU_W 			    0.005
#define GAMMA 			    0.5
#define DELTA 			    0.1
#define ONE_OVER_DELTA 	    10.0
#define MU_2 			    0.01
#define ALPHA 			    0.0
#define NUM_ITER 		    2000
#define ALPHA_W 		    0.0
#define BETA 			    0.01

// Slave node memory addresses
#define XT_MEM_ADDR 0x4000
#define WK_MEM_ADDR 0x4230
#define UP_WK_MEM_ADDR 0x4460
#define NU_OPT_MEM_ADDR 0x4690
#define NU_K0_MEM_ADDR 0x4920
#define NU_K1_MEM_ADDR 0x5150
#define NU_K2_MEM_ADDR 0x5380

// Master node memory addresses
#define XT_SHMEM_ADDR 0x8f000000
#define DONE_MEM_ADDR_0 0x4000
#define DONE_MEM_ADDR_1 0x4010
#define DONE_MEM_ADDR_2 0x4020

#endif // COMMON_H_INCLUDED
