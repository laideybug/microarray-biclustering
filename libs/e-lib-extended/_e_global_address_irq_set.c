#include "_e_global_address_irq_set.h"

inline void _e_global_address_irq_set(unsigned row, unsigned col, e_irq_type_t irq) {
    unsigned *ilatst;
    ilatst = (unsigned *)_e_get_global_address_on_chip(row, col, (void *)E_REG_ILATST);

    *ilatst = 1 << (irq - E_SYNC);

    return;
}
