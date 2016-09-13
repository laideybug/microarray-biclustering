#include "e_sync.h"

volatile e_barrier_t barriers[M_N]    SECTION("section_core");
         e_barrier_t *tgt_bars[M_N]   SECTION("section_core");
