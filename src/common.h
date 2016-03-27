#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdlib.h>

#define USE_MASTER_NODE

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

// Shared memory base address
#define SHMEM_ADDR          0x8f000000

// Node memory addresses
#define XT_MEM_ADDR         0x4000
#define WK_MEM_ADDR         0x4230
#define UP_WK_MEM_ADDR      0x4460
#define NU_OPT_MEM_ADDR     0x4690
#define NU_K0_MEM_ADDR      0x4920
#define NU_K1_MEM_ADDR      0x5150
#define NU_K2_MEM_ADDR      0x5380
#define NU_K2_MEM_ADDR      0x5380

// Master node memory addresses
#define DONE_MEM_ADDR_0     0x4000
#define DONE_MEM_ADDR_1     0x4004
#define DONE_MEM_ADDR_2     0x4008

#define INF_CLKS_MEM_ADDR_0 0x4100
#define INF_CLKS_MEM_ADDR_1 0x4104
#define INF_CLKS_MEM_ADDR_2 0x4108
#define UP_CLKS_MEM_ADDR_0  0x4112
#define UP_CLKS_MEM_ADDR_1  0x4116
#define UP_CLKS_MEM_ADDR_2  0x4120

// Core speeds
#define E_MHZ 600
#define A_MHZ 667

#endif // COMMON_H_INCLUDED
