/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General 
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** map246.c
**
** mapper 246 interface
** $Id: map246.c,v 1.0 2025/01/12 23:41:11 delguoqing Exp $
*/

#include "nes.h"

/* mapper 246: G0151-1, used in Fengshenbang: Fumosantaizi */

static void map246_init(void)
{
   mmc_bankrom(32, 0x8000, MMC_LASTBANK);
   mmc_bankvrom(8, 0x0000, MMC_LASTBANK);
}

static void map246_write(uint32 address, uint8 value)
{
   if (address >= 0x6000 && address <= 0x6003)
   {
      mmc_bankrom(8, 0x8000 + (address - 0x6000) * 0x2000, value & 0x3F);
   }
   else if (address >= 0x6004 && address <= 0x6007)
   {
      mmc_bankvrom(2, (address - 0x6004) * 0x800, value & 0xFF);
   }
   else if (address >= 0x6800 && address <= 0x6FFF)
   {
      nes_getcontextptr()->cpu->mem_page[address >> 12][address & 0xFFF] = value;
   }
   else
   {
      printf("map246: unhandled write [%p] = 0x%x\n", (void*)address, value);
   }
}

static uint8 map246_read(uint32 address)
{
    return nes_getcontextptr()->cpu->mem_page[address >> 12][address & 0xFFF];
}

static map_memwrite map246_memwrite[] =
{
   { 0x6000, 0x6007, map246_write },
   { 0x6800, 0x6FFF, map246_write },
   { 0x8000, 0xFFFF, map246_write },
   {     -1,     -1, NULL }
};

static map_memread map246_memread[] = 
{
   { 0x6800, 0x6FFF, map246_read },
   {     -1,     -1, NULL }
};

mapintf_t map246_intf = 
{
   246, /* mapper number */
   "G0151-1", /* mapper name */
   map246_init, /* init routine */
   NULL, /* vblank callback */
   NULL, /* hblank callback */
   NULL, /* get state (snss) */
   NULL, /* set state (snss) */
   map246_memread, /* memory read structure */
   map246_memwrite, /* memory write structure */
   NULL /* external sound device */
};