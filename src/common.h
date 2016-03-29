#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdlib.h>
#include <string.h>

// Configuration macros
#define USE_MASTER_NODE
#ifdef USE_MASTER_NODE
#define MASTER_NODE         1
#else
#define MASTER_NODE         0
#endif

#define USE_DMA
#ifdef USE_DMA
#define e_memcopy(dst, src, size) e_dma_copy(dst, src, size)
#else
#define e_memcopy(dst, src, size) memcpy(dst, src, size)
#endif

//#define USE_BARRIER
#ifdef USE_BARRIER
#undef USE_MASTER_NODE
#endif // BARRIERS CANNOT BE USED WITH MASTER NODE CONFIGURATION

// Program constants
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
#define SET_FLAG            0x00000001
#define CLEAR_FLAG          0x00000000

// Core speeds - cycles per second
#define E_CYCLES            600000000.0   // 600MHz
#define A_CYCLES            667000000.0   // 667Mhz

// Shared memory base address
#define SHMEM_ADDR          0x8f000000

// Node memory addresses
#define XT_MEM_ADDR         0x4200
#define WK_MEM_ADDR         0x4430
#define UP_WK_MEM_ADDR      0x4660
#define NU_OPT_MEM_ADDR     0x4890
#define NU_K0_MEM_ADDR      0x5120
#define NU_K1_MEM_ADDR      0x5350
#define NU_K2_MEM_ADDR      0x5580
#define NU_K_FLAG_ADDR      0x5900
#define NU_MEM_OFFSET       0x0230

// Master node macros and addresses
#define MASTER_ADDR_NUM     5
#define INF_CLKS_MEM_ADDR   0x4000
#define INF_CLKS_MEM_ADDR   0x4000
#define UP_CLKS_MEM_ADDR    0x4200
#define READY_MEM_ADDR      0x4400
#define DONE_MEM_ADDR       0x4600

#endif // COMMON_H_INCLUDED
