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
** map163.c
**
** mapper 163 interface
** $Id: map163.c,v 1.2 2001/04/27 14:37:11 neil Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <noftypes.h>
#include <nes_mmc.h>
#include <nes_ppu.h>
#include <nes.h>
#include <libsnss.h>

static struct mapper163Data state;

static void map163_update_rom_bank()
{
    unsigned char bank = state.PRGBank;
    if (!state.useA15A16From5000)
    {
        bank |= 0x3;
    }
    mmc_bankrom(32, 0x8000, bank);
}

static void map163_update_vrom_bank()
{
    if (!state.chrRamSwitchEnabled)
    {
        mmc_bankvrom(8, 0x0000, 0);
    }
    else
    {
        mmc_bankvrom(4, state.latchA9 * 0x1000, 0);
        mmc_bankvrom(4, state.latchA9 * 0x1000, 1);
    }
}

static void map163_write(uint32 address, uint8 value)
{
    unsigned char bit0 = value & 1;
    unsigned char bit1 = (value >> 1) & 1;

    if (address >= 0x6000 && address <= 0x7FFF) 
    {
        nes_getcontextptr()->cpu->mem_page[address >> 12][address & 0xFFF] = value;
        return;
    }    

    // bit swap according to mode
    if (address >= 0x5000 && address <= 0x5200 && state.swapLast2Bit)
    {
        value = (value & 0xFC) | (bit0 << 1) | bit1;
    }

    switch (address)
    {
    case 0x5000:
        state.chrRamSwitchEnabled = (value >> 7) & 1;
        state.PRGBank = (state.PRGBank & 0xF0) | (value & 0xF);
        map163_update_rom_bank();
        map163_update_vrom_bank();
        break;

    case 0x5200:
        state.PRGBank = (state.PRGBank & 0xF) | ((value & 0x3) << 4);
        map163_update_rom_bank();
        break;

    case 0x5100:
        state.latchE = (value & 1);
        state.latchF = ((value >> 2) & 1);
        break;

    case 0x5101:
        if ((value & 1) == 1)
        {
            state.latchF = 1 - state.latchF;
        }
        break;

    case 0x5300:
        state.swapLast2Bit = value & 1;
        state.useA15A16From5000 = (value >> 2) & 1;
        map163_update_rom_bank();
        break;
    
    default:
        break;
    }
}

static uint8 map163_read(uint32 address)
{
    if (address >= 0x6000 && address <= 0x7FFF)
    {
        return nes_getcontextptr()->cpu->mem_page[address >> 12][address & 0xFFF];
    }
    else if (address == 0x5500 || address == 0x5501)
    {
        return (1 - state.latchF) << 2;
    }
    else
    {
        printf("map163: read: address=%p\n", (void*)address);
        return 0;
    }
}

static void map163_latchfunc(uint32 address, uint8 value)
{
    // detect a13 rise and latch a9 if needed
    unsigned char a13 = (address >> 13) & 1;
    if (state.prevPPUA13 == 0 && a13 == 1)
    {
        state.latchA9 = (address >> 9) & 1;
        map163_update_vrom_bank();
    }
    state.prevPPUA13 = a13;
}

static void map163_init(void)
{
    memset(&state, 0, sizeof(state));
    ppu_setlatchfunc(map163_latchfunc);
    map163_update_rom_bank();
    map163_update_vrom_bank();
}

static void map163_getstate(SnssMapperBlock *state)
{
    memcpy(&(state->extraData.mapper163), &state, sizeof(state));
}

static void map163_setstate(SnssMapperBlock *state)
{
    memcpy(&state, &(state->extraData.mapper163), sizeof(state));
}

static map_memwrite map163_memwrite[] =
{
   { 0x5000, 0x5300, map163_write },
   { 0x6000, 0x7FFF, map163_write },
   {     -1,     -1, NULL }
};

static map_memread map163_memread[] =
{
   { 0x5500, 0x5501, map163_read },
   { 0x6000, 0x7FFF, map163_read },
   {     -1,     -1, NULL }
};

mapintf_t map163_intf =
{
   163, /* mapper number */
   "FC-001", /* mapper name */
   map163_init, /* init routine */
   NULL, /* vblank callback */
   NULL, /* hblank callback */
   map163_getstate, /* get state (snss) */
   map163_setstate, /* set state (snss) */
   map163_memread, /* memory read structure */
   map163_memwrite, /* memory write structure */
   NULL /* external sound device */
};