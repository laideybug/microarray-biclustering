#include "e_microarray_biclustering_utils.h"
#include "common.h"
#include "e_coreid.h"
#include "e_regs.h"

void e_global_mutex_init(unsigned row, unsigned col, e_mutex_t *mutex, e_mutexattr_t *attr) {
    e_mutex_t *gmutex;
    // Unused
    (void)attr;

    gmutex = (e_mutex_t *)e_get_global_address_on_chip(row, col, mutex);

    *gmutex = 0x0;

    return;
}

void e_global_mutex_lock(unsigned row, unsigned col, e_mutex_t *mutex) {
	e_mutex_t *gmutex;
	register unsigned coreid, offset;

	coreid = e_get_coreid();
	gmutex = (e_mutex_t *)e_get_global_address_on_chip(row, col, mutex);
	offset = 0x0;

	do {
		__asm__ __volatile__("testset  %[r0], [%[r1], %[r2]]" : [r0] "+r" (coreid) : [r1] "r" (gmutex), [r2] "r" (offset));
	} while (coreid != 0);

	return;
}


unsigned e_global_mutex_trylock(unsigned row, unsigned col, e_mutex_t *mutex) {
	e_mutex_t *gmutex;
    register unsigned coreid, offset;

	coreid = e_get_coreid();
	gmutex = (e_mutex_t *)e_get_global_address_on_chip(row, col, mutex);
	offset = 0x0;

	__asm__ __volatile__("testset %[r0], [%[r1], %[r2]]" : [r0] "+r" (coreid) : [r1] "r" (gmutex), [r2] "r" (offset));

	return coreid;
}

void e_global_mutex_unlock(unsigned row, unsigned col, e_mutex_t *mutex) {
	e_mutex_t *gmutex;

	gmutex = (e_mutex_t *)e_get_global_address_on_chip(row, col, mutex);

	*gmutex = 0x0;

	return;
}

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
