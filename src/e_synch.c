#include "e_synch.h"

volatile e_barrier_t barriers[M*N]    SECTION("section_core");
         e_barrier_t *tgt_bars[M*N]   SECTION("section_core");
