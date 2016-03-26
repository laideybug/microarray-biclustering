#include "static_buffers.h"

volatile e_barrier_t barriers[N]    SECTION("section_core");
         e_barrier_t *tgt_bars[N]   SECTION("section_core");
