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

/* $Id: flash.h 1110 2008-05-06 09:38:42Z jaguarondi $ */

/** \defgroup flash Flash memory
    \ingroup flash

   This module control all the functions to read / write the flash memory.
*/

/** \file flash.h
    \ingroup flash
*/
/** \file flash.c
    \ingroup flash
*/
#ifndef FLASH_H
#define FLASH_H

/** \name Flash programming states
  @{ */
enum {
    DETECT_INDEXES = 0,
    PROG_INIT,
    PROGRAMMING,
    WAITING_STATE,
    PROG_TOC,
    PROG_END,
};
/* @} */

/** \name No sound in frame timeout 
  @{ */
#define STOP_FRAME_NUMBER  10
#define TTS_TIMEOUT 10 
#define START_FRAME_NUMBER  100
/* @} */

/** \name Flash programming status
 @{ */
/*
enum {
    STANDBY = 0,
    IN_PROGRESS,
    WAITING_FOR_CONFIRMATION,
    WRITE_TOC,
    ERASING_LAST_SOUND,
    FLASH_FULL,
    NO_SOUND,
};
*/
/* @} */

extern void programming(void);
extern void playSound(void);
extern void erase(void);
extern uint8_t readFlashNumber(void);
extern uint8_t readLastBlock(uint8_t num);
extern void enter_deep_sleep(void);
extern void leave_deep_sleep(void);

/** start / end flash states flag */
extern uint8_t flash_state;
/** state machine variable */
extern uint8_t f_state;
/** sound number to be played */
extern uint8_t soundNum;
#endif
