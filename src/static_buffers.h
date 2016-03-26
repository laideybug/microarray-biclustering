#ifndef STATIC_BUFFERS_H_INCLUDED
#define STATIC_BUFFERS_H_INCLUDED

#include <e-lib.h>
#include "common.h"

extern volatile e_barrier_t barriers[N];
extern          e_barrier_t *tgt_bars[N];

#endif // STATIC_BUFFERS_H_INCLUDED
