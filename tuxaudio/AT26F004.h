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

/* $Id: AT26F004.h 1112 2008-05-06 09:54:21Z jaguarondi $ */

/** \defgroup at26f004 AT26F004 flash memory 
    \ingroup at26f004

    This module contains all specific definitions and functions to access the flash memory.
    */

/** \file AT26F004.h
    \ingroup at26f004
    \brief AT26F004 flash memory header
    */
 
#ifndef AT26F004_H
#define AT26F004_H

#include "hardware.h"

/** \file AT26F004.c
    \ingroup at26f004
    \brief AT26F004 functions
    */


/** 
 * \name Read opcodes 
    @{ */
#define READ_ARRAY          0x0B
#define READ_ARRAY_LOW_F    0x03
/*    @} */
/** \name Erase opcodes 
 *@{ */
#define BLOCK_ERASE_4K      0x20
#define BLOCK_ERASE_32K     0x52
#define BLOCK_ERASE_64K     0xD8
#define CHIP_ERASE          0x60
/*    @} */

/** \name Program opcodes 
 * @{ */
#define BYTE_PROGRAM        0x02
#define SEQU_PROGRAM        0xAF
/* @} */

/** \name Sectors managment opcodes
  * @{ */
#define WRITE_EN            0x06
#define WRITE_DIS           0x04
#define PROTECT_SECTOR      0x36
#define UNPROTECT_SECTOR    0x39
#define READ_SECT_PROTECT   0x3C
/* @} */
/** \name Status command 
 * @{ */
#define READ_STATUS_REG     0x05
#define WRITE_STATUS_REG    0x01
/* @} */
/** \name Misc. opcodes
 * @{ */
#define READ_MANUFACT       0x9F
#define DEEP_POWER_MODE     0xB9
#define RESUME_DEEP_MODE    0xAB

#define NOP                 0x00
/* @} */

/** 
 * \name AT26F004 status register masks 
 * 
 *  @{ */
#define BUSY                0x01
#define WEL                 0X02
#define SWP                 0x0C
#define WPP                 0x10
#define RES                 0x20
#define SPM                 0x40
#define SPRL                0x80
/*! @} */

/** \name Flash sector adresses
 * @{ */
#define SECTOR0  0x00, 0x00, 0x00
#define SECTOR1  0x01, 0x00, 0x00
#define SECTOR2  0x02, 0x00, 0x00
#define SECTOR3  0x03, 0x00, 0X00
#define SECTOR4  0x04, 0x00, 0x00
#define SECTOR5  0x05, 0x00, 0x00
#define SECTOR6  0x06, 0X00, 0x00
#define SECTOR7  0x07, 0x00, 0x00
#define SECTOR8  0x07, 0x80, 0x00
#define SECTOR9  0x07, 0xA0, 0x00
#define SECTOR10 0x07, 0xC0, 0x00

/* @} */


/**
 * \name Flash TOP address
 * The flash memory address range is 0x000000 - [TOP_A2 TOP_A1 TOP_A0]
 * The AT26F004 has an address range of 0x000000 - 0x7FFFFF
 * @{ */

/** High byte of the TOP address. */
#define TOP_A2 0x07
/** Middle byte of the TOP address. */
#define TOP_A1 0xFF
/** Low byte of the TOP address. */
#define TOP_A0 0xFF
/*! @} */

/** \name Status access functions 
 * @{ */
extern uint8_t read_status(void);
extern void write_status(uint8_t const status);
/* @} */
/** \name Writing functions 
 * @{ */
extern void write_enable(void);
extern void write_disable(void);
extern void program_flash(uint8_t const ad2, uint8_t const ad1, uint8_t const ad0,
                          uint8_t const data);
/* @} */
/** \name Reading function 
 * @{ */
extern uint8_t read_data(uint8_t const ad2, uint8_t const ad1, uint8_t const ad0);
/* @} */
/** \name Misc. functions 
 * @{ */
extern void erase_flash(void);
extern void blockErase(uint8_t first_block, uint8_t last_block);
extern void unprotect_sector(uint8_t const ad2, uint8_t const ad1,
                             uint8_t const ad0);
/* @} */

#endif
