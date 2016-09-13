#ifndef E_SYNC_H_INCLUDED
#define E_SYNC_H_INCLUDED

#include <e-lib.h>
#include "common.h"

extern volatile e_barrier_t barriers[M_N];
extern          e_barrier_t *tgt_bars[M_N];

#endif // E_SYNC_H_INCLUDED
