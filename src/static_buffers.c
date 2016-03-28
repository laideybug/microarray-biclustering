#include "static_buffers.h"

#ifdef USE_MASTER_NODE
volatile e_barrier_t barriers[N+1]    SECTION("section_core");
         e_barrier_t *tgt_bars[N+1]   SECTION("section_core");
#else
volatile e_barrier_t barriers[N]    SECTION("section_core");
         e_barrier_t *tgt_bars[N]   SECTION("section_core");
#endif
