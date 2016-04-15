#include "_e_get_global_address_on_chip.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
inline void *_e_get_global_address_on_chip(unsigned row, unsigned col, const void *ptr) {
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
