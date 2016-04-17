#include "_e_global_mutex.h"
#include "_e_get_global_address_on_chip.h"

inline void _e_global_mutex_unlock(unsigned row, unsigned col, e_mutex_t *mutex) {
	e_mutex_t *gmutex;

	gmutex = (e_mutex_t *)_e_get_global_address_on_chip(row, col, mutex);

	*gmutex = 0x0;

	return;
}
