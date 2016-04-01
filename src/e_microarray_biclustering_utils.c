#include "e_microarray_biclustering_utils.h"
#include "common.h"
#include "e_regs.h"

void e_global_address_irq_set(unsigned row, unsigned col, e_irq_type_t irq) {
    unsigned *ilatst;
    ilatst = (unsigned *)e_get_global_address_on_chip(row, col, (void *)E_REG_ILATST);

    *ilatst = 1 << (irq - E_SYNC);

    return;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
void *e_get_global_address_on_chip(unsigned row, unsigned col, const void *ptr) {
    unsigned uptr;
    unsigned coreid;

    if (((unsigned) ptr) & 0xfff00000) {
        uptr = (unsigned)ptr;
        return (void *)uptr;
    }

    coreid = (row + PLATFORM_ROW) * 0x40 + col + PLATFORM_COL;
    uptr = (unsigned)ptr;
    uptr = (coreid << 20) | uptr;

    return (void *)uptr;
}
#pragma GCC diagnostic pop
