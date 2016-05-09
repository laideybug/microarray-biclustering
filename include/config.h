#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/*==============================================*/
/*=============Configuration Modes==============*/
/*==============================================*/

// Algorithm modes - CHOOSE ONE, comment out the rest
//#define DISTRIBUTED
//#define BATCH_DISTRIBUTED
#define MULTI_DISTRIBUTED

// Network configuration options - comment out as needed
#define USE_MASTER_NODE
//#define USE_DMA


/*==============================================*/
/*===========Configuration Constants============*/
/*==============================================*/

// Network coordinates - do not change
#define NETWORK_ORIGIN_ROW  0
#define NETWORK_ORIGIN_COL  0

#define MASTER_NODE_ROW     0
#define MASTER_NODE_COL     3

// Program constants
#define IN_ROWS			    56
#define ONE_OVER_IN_ROWS    0.017857143
#define IN_COLS			    12625
#define ONE_OVER_IN_COLS    7.920792079e-5

#if 	defined DISTRIBUTED
#undef	BATCH_DISTRIBUTED
#undef	MULTI_DISTRIBUTED
#define WK_ROWS			    56
#define M 				    1
#define N 				    3
#define M_N 				3
#define ONE_OVER_M 		    1
#define ONE_OVER_N 		    0.333333333
#define ONE_OVER_M_N 		0.333333333
#define BATCH_STARTS		1
#define BATCH_STARTS_N      3
#define BATCH_TOGGLE		0
#define AGENT_BIN_PATH		"e_mb.srec"
#define MASTER_BIN_PATH		"e_mb_master.srec"
#define MODE        		"Distributed"

#elif 	defined BATCH_DISTRIBUTED
#undef	DISTRIBUTED
#undef	MULTI_DISTRIBUTED
#define WK_ROWS			    56
#define M 				    4
#define N 				    3
#define M_N 				12
#define ONE_OVER_M 		    0.25
#define ONE_OVER_N 		    0.333333333
#define ONE_OVER_M_N 		0.083333333
#define BATCH_STARTS		4
#define BATCH_STARTS_N      12
#define BATCH_TOGGLE		1
#define AGENT_BIN_PATH		"e_mb_batch.srec"
#define MASTER_BIN_PATH		"e_mb_batch_master.srec"
#define MODE        		"Batch-distributed"

#elif 	defined MULTI_DISTRIBUTED
#undef	DISTRIBUTED
#undef	BATCH_DISTRIBUTED
#define WK_ROWS			    14
#define M 				    4
#define N 				    3
#define M_N 				12
#define ONE_OVER_M 		    0.25
#define ONE_OVER_N 		    0.333333333
#define ONE_OVER_M_N 		0.083333333
#define BATCH_STARTS		1
#define BATCH_STARTS_N      3
#define BATCH_TOGGLE		0
#define AGENT_BIN_PATH		"e_mb_multi.srec"
#define MASTER_BIN_PATH		"e_mb_multi_master.srec"
#define MODE        		"Multi-distributed"
#endif

#ifdef 	USE_MASTER_NODE
#define MASTER_NODE         1
#else
#define MASTER_NODE         0
#endif

#ifdef 	USE_DMA
#define e_memcopy(dst, src, size) e_dma_copy(dst, src, size)
#else
#define e_memcopy(dst, src, size) memcpy(dst, src, size)
#endif

#endif // CONFIG_H_INCLUDED
