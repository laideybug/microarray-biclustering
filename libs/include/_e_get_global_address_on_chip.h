#ifndef _E_GET_GLOBAL_ADDRESS_ON_CHIP_H_INCLUDED
#define _E_GET_GLOBAL_ADDRESS_ON_CHIP_H_INCLUDED

#define PLATFORM_ROW 32 // Platform row number of core (0,0) on Parallella-16 Epiphany
#define PLATFORM_COL 8  // Platform col number of core (0,0) on Parallella-16 Epiphany

void *_e_get_global_address_on_chip(unsigned row, unsigned col, const void *ptr);

#endif // _E_GET_GLOBAL_ADDRESS_ON_CHIP_H_INCLUDED
