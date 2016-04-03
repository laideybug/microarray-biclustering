#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdlib.h>
#include <string.h>

// Configuration macros
//#define USE_MASTER_NODE
#define USE_DMA

#ifdef USE_MASTER_NODE
#define MASTER_NODE         1
#else
#define MASTER_NODE         0
#endif

#ifdef USE_DMA
#define e_memcopy(dst, src, size) e_dma_copy(dst, src, size)
#else
#define e_memcopy(dst, src, size) memcpy(dst, src, size)
#endif

// Platform constants
#define PLATFORM_ROW        32
#define PLATFORM_COL        8

// Program constants
#define IN_ROWS			    56
#define ONE_OVER_IN_ROWS    0.017857143
#define IN_COLS			    12625
#define ONE_OVER_IN_COLS    7.920792079e-5
#define WK_ROWS			    14
#define M 				    4
#define N 				    3
#define ONE_OVER_N 		    0.333333333
#define ONE_OVER_M_N 		0.083333333
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
#define SET_FLAG            0x00000001
#define CLEAR_FLAG          0x00000000

// Core speeds - cycles per second
#define E_CYCLES            600e6   // 600MHz
#define ONE_OVER_E_CYCLES   1.66666667e-9
#define A_CYCLES            667e6   // 667Mhz
#define ONE_OVER_A_CYCLES   1.49925037e-9

// Shared memory bast address
#define SHMEM_ADDR          0x8f000000

// Node memory addresses
#define XT_MEM_ADDR         0x4000
#define WK_MEM_ADDR         0x4230
#define UP_WK_MEM_ADDR      0x4460
#define NU_OPT_MEM_ADDR     0x4690
#define NU_K0_MEM_ADDR      0x4920
#define NU_K1_MEM_ADDR      0x5150
#define NU_K2_MEM_ADDR      0x5380
#define INC_SCAL_MEM_ADDR   0x5800
#define INC_RMS_MEM_ADDR    0x5850
#define NU_MEM_OFFSET       0x0230

// Master node macros and addresses
#define MASTER_NODE_ROW     0
#define MASTER_NODE_COL     3
#define MASTER_ADDR_NUM     5		// The number of items returned by the master node
#define INF_CLKS_MEM_ADDR   0x4000
#define UP_CLKS_MEM_ADDR    0x4200
#define READY_MEM_ADDR      0x4400
#define DONE_MEM_ADDR       0x4600

#endif // COMMON_H_INCLUDED
