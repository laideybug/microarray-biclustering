#ifndef E_MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED
#define E_MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED

#include "e_ic.h"
#include "e_mutex.h"

void e_global_mutex_init(unsigned row, unsigned col, e_mutex_t *mutex, e_mutexattr_t *attr);
void e_global_mutex_lock(unsigned row, unsigned col, e_mutex_t *mutex);
unsigned e_global_mutex_trylock(unsigned row, unsigned col, e_mutex_t *mutex);
void e_global_mutex_unlock(unsigned row, unsigned col, e_mutex_t *mutex);
void e_global_address_irq_set(unsigned row, unsigned col, e_irq_type_t irq);
void *e_get_global_address_on_chip(unsigned row, unsigned col, const void *ptr);

#endif // E_MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED
