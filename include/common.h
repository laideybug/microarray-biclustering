#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <string.h>
#include "config.h"

/*==============================================*/
/*==============Program Constants===============*/
/*==============================================*/

// Algorithm constants
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
#define DATA_PATH			"../data/data.txt"

// Core speeds - cycles per second
#define E_CYCLES            600e6   // 600MHz (Epiphany)
#define ONE_OVER_E_CYCLES   1.66666667e-9
#define A_CYCLES            667e6   // 667Mhz (ARM)
#define ONE_OVER_A_CYCLES   1.49925037e-9

// Shared memory bast address
#define SHMEM_ADDR          0x8f000000

// Node memory addresses
#define XT_MEM_ADDR         0x4000
#define WK_MEM_ADDR         0x4230
#define UP_WK_MEM_ADDR      0x4460
#define NU_K0_MEM_ADDR  	0x4690
#define NU_K1_MEM_ADDR  	0x4920
#define NU_K2_MEM_ADDR  	0x5150
#define NU_OPT_K0_MEM_ADDR  0x5380
#define NU_OPT_K1_MEM_ADDR  0x5610  // Only used in BATCH_DISTRIBUTED mode
#define NU_OPT_K2_MEM_ADDR  0x5840  // Only used in BATCH_DISTRIBUTED mode
#define NU_OPT_K3_MEM_ADDR  0x6070  // Only used in BATCH_DISTRIBUTED mode
#define INC_SCAL_MEM_ADDR   0x5610  // Only used in MULTI_DISTRIBUTED mode
#define INC_RMS_MEM_ADDR    0x5620  // Only used in MULTI_DISTRIBUTED mode
#define NU_MEM_OFFSET       0x0230

// Master node macros and addresses
#define MASTER_BENCH_NUM    6		// The number of benchmark values returned by the master node
#define INF_CLKS_MEM_ADDR   0x4000
#define UP_CLKS_MEM_ADDR    0x4200
#define READY_MEM_ADDR      0x4400
#define DONE_MEM_ADDR       0x4600
#define DONE_MUTEX_MEM_ADDR 0x4800

#endif // COMMON_H_INCLUDED
