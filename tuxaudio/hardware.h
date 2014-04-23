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

/* $Id: hardware.h 2250 2008-10-07 14:26:13Z jaguarondi $ */

#include <avr/io.h>

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/** VCC mask. */
#define VCC_MK _BV(PD6)
/** VCC PIN. */
#define VCC_PIN PIND
/** VCC PORT. */
#define VCC_PT PORTD
/** VCC DDR. */
#define VCC_DDR DDRD

/** RF reset mask. */
#define RF_RESET_MK _BV(PB7)
/** RF reset PIN. */
#define RF_RESET_PIN PINB
/** RF reset PORT. */
#define RF_RESET_PT PORTB
/** RF reset DDR. */
#define RF_RESET_DDR DDRB
/** RF online mask. */
#define RF_ONLINE_MK _BV(PB6)
/** RF online PIN. */
#define RF_ONLINE_PIN PINB
/** RF online PORT. */
#define RF_ONLINE_PT PORTB
/** RF online DDR. */
#define RF_ONLINE_DDR DDRB
/** RF chip select mask. */
#define RF_CS_MK _BV(PB2)
/** RF chip select PIN. */
#define RF_CS_PIN PINB
/** RF chip select PORT. */
#define RF_CS_PT PORTB
/** RF chip select DDR. */
#define RF_CS_DDR DDRB

#define rf_select() (PORTB &= ~_BV(PB2))
#define rf_unselect() (PORTB |= _BV(PB2))

/* Flash memory port */
#define FLASH_PORT PORTB
#define FLASH_CS_PIN _BV(PB1)
#define FLASH_HOLD_PIN _BV(PB0)
/* Flash memory commands */
#define flash_select() (FLASH_PORT &= ~FLASH_CS_PIN)
#define flash_unselect() (FLASH_PORT |= FLASH_CS_PIN)
#define flash_onhold() (FLASH_PORT &= ~FLASH_HOLD_PIN)
#define flash_enable() (FLASH_PORT |= FLASH_HOLD_PIN)

/* Power management */
#define POWER_DDR           DDRD        /* VCC power switch */
#define POWER_MK            _BV(PD6)
#define POWER_PT            PORTD

/* Audio mute */
#define MUTE_DDR            DDRD        /* amp _SHDN pin */
#define MUTE_MK             _BV(PD7)
#define MUTE_PT             PORTD

#define AUDIO_OUT_DDR       DDRD        /* PWM output compare pin */
#define AUDIO_OUT_MK        _BV(PD5)
#define AUDIO_OUT_PT        PORTD

static __inline__ void unmute_amp(void)
{
    (MUTE_PT |= MUTE_MK);
    (AUDIO_OUT_DDR |= AUDIO_OUT_MK);
}
static __inline__ void mute_amp(void)
{
    (MUTE_PT &= ~MUTE_MK);
    (AUDIO_OUT_DDR &= ~AUDIO_OUT_MK);
}

#endif /* _HARDWARE_H_ */
