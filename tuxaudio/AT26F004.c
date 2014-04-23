/*
 * TUXAUDIO - Firmware for the 'audio' CPU of tuxdroid
 * Copyright (C) 2007 C2ME S.A. <tuxdroid@c2me.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* $Id: AT26F004.c 1112 2008-05-06 09:54:21Z jaguarondi $ */

#include <avr/io.h>
#include "AT26F004.h"
#include "hardware.h"
#include "spi.h"

static void unprotect_sectors(void);
/**
 * \ingroup at26f004
 * \brief initialize a table with the sector adresses values
 */
static uint8_t sector_adress[11][3] = {
    {SECTOR0},
    {SECTOR1},
    {SECTOR2},
    {SECTOR3},
    {SECTOR4},
    {SECTOR5},
    {SECTOR6},
    {SECTOR7},
    {SECTOR8},
    {SECTOR9},
    {SECTOR10}
};

/**
 * \ingroup at26f004

   \brief This function set the write enable flag of the flash memory.
   */
uint8_t read_status(void)
{
    uint8_t status;

    flash_select();
    spiSend(READ_STATUS_REG); /* Send Read Status Command */
    status = spiSend(NOP); /* Read status on spi */
    flash_unselect();
    return status;
}
/**
 * \ingroup at26f004

   \brief This function set the write enable flag of the flash memory.
   */
void write_enable(void)
{
    flash_select();
    spiSend(WRITE_EN); /* Send Write Enable Command */
    flash_unselect();
}
/**
 * \ingroup at26f004

   \brief This function clear the write enable flag of the flash memory.
   */
void write_disable(void)
{
    flash_select();
    spiSend(WRITE_DIS); /* Send Write Disable Command */
    flash_unselect();
}

/**
 * \ingroup at26f004

   \brief This function write into the flash memory status register.
   */
void write_status(uint8_t const status)
{
    flash_select();
    spiSend(WRITE_STATUS_REG); /* Send Write Status Command */
    spiSend(status); /* Send status */
    flash_unselect();
}
/**
 * \ingroup at26f004
   \param ad2 high address part
   \param ad1 medium adress part
   \param ad0 lower adress part

   \brief This function unprotect a sector.
   */
void unprotect_sector(uint8_t const ad2, uint8_t const ad1, uint8_t const ad0)
{
    flash_select();
    spiSend(UNPROTECT_SECTOR); /* Send unprotect sector command */
    /* Send Adress */
    spiSend(ad2);
    spiSend(ad1);
    spiSend(ad0);
    flash_unselect();
}
/**
 * \ingroup at26f004

   \brief This function erase the entire memory.
   */
void erase_flash(void)
{

    unprotect_sectors();
    write_enable(); /* Enable the writing */

    flash_select();
    spiSend(CHIP_ERASE); /* Send Erase Bulk command */
    flash_unselect();
}

static void unprotect_sectors(void)
{
    uint8_t i;
    write_status(0x00); /* Disable sector protection register */
    for (i=0; i<=10; i++)
    {
        write_enable(); /* Enable the writing */
        unprotect_sector(sector_adress[i][0], sector_adress[i][1],sector_adress[i][2]);
    }
}
/**
 * \ingroup at26f004
   \param ad2 high address part
   \param ad1 medium adress part
   \param ad0 lower adress part

   \brief This function write a byte in the flash memory.
   */
void program_flash(uint8_t const ad2, uint8_t const ad1, uint8_t const ad0,
                   uint8_t const data)
{
    while (read_status() & BUSY) ; /* Wait Page Program Cycle */

    write_enable();
    flash_select();
    spiSend(BYTE_PROGRAM); /* Send Page Byte Command */
    /* Send adress */
    spiSend(ad2);
    spiSend(ad1);
    spiSend(ad0);
    spiSend(data); /* Write data in flash */
    flash_unselect();
}

/**
 * \ingroup at26f004
   \param ad2 high address part
   \param ad1 medium adress part
   \param ad0 lower adress part
   \return Data read
   \brief This function read a single byte in the flash memory.
   */
uint8_t read_data(uint8_t const ad2, uint8_t const ad1, uint8_t const ad0)
{
    uint8_t data1;

    flash_select();
    spiSend(READ_ARRAY_LOW_F); /* Send Read Page Command */
    /* Send address */
    spiSend(ad2);
    spiSend(ad1);
    spiSend(ad0);
    data1 = spiSend(NOP); /* Wait response */
    flash_unselect();

    return data1;
}

/**
 * \ingroup at26f004
   \param first_block The first block to erase
   \param last_block The last block to erase
   \brief This function erase a specified area in the memory. The memory will be erased by 4kB's blocks.
   */
void blockErase(uint8_t first_block, uint8_t last_block)
{
    uint8_t diff, ad0, ad1;

    // unprotect all sectors
    unprotect_sectors();

    // diff is the number of block to erase
    diff = last_block - first_block + 1;
    while(diff)
    {
        // Erase a 4kB block
        write_enable();
        flash_select();
        ad0 = (last_block >> 4);
        ad1 = (last_block << 4);
        spiSend(BLOCK_ERASE_4K); /* Send Erase Bulk command */
        spiSend(ad0);
        spiSend(ad1);
        spiSend(0x00);
        flash_unselect();

        diff --;
        last_block --;
        while (read_status() & BUSY);
    }
}

