#ifndef _E_GLOBAL_ADDRESS_IRQ_SET_H_INCLUDED
#define _E_GLOBAL_ADDRESS_IRQ_SET_H_INCLUDED

#include "_e_get_global_address_on_chip.h"
#include "e_ic.h"
#include "e_regs.h"

void _e_global_address_irq_set(unsigned row, unsigned col, e_irq_type_t irq);

#endif // _E_GLOBAL_ADDRESS_IRQ_SET_H_INCLUDED
