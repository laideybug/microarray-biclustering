#ifndef STATIC_BUFFERS_H_INCLUDED
#define STATIC_BUFFERS_H_INCLUDED

#include <e-lib.h>
#include "common.h"

#ifdef USE_MASTER_NODE
extern volatile e_barrier_t barriers[N+1];
extern          e_barrier_t *tgt_bars[N+1];
#else
extern volatile e_barrier_t barriers[N];
extern          e_barrier_t *tgt_bars[N];
#endif

#endif // STATIC_BUFFERS_H_INCLUDED
