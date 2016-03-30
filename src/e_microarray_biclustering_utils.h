#ifndef E_MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED
#define E_MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED

#include "e_ic.h"

void e_global_address_irq_set(unsigned row, unsigned col, e_irq_type_t irq);
void *e_get_global_address_on_chip(unsigned row, unsigned col, const void *ptr);

#endif // E_MICROARRAY_BICLUSTERING_UTILS_H_INCLUDED
