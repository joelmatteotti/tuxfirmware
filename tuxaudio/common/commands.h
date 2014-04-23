/*
 * TUXDEFS - Common defines used by the firmware and daemon of tuxdroid
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

/* $Id: commands.h 2271 2008-10-08 10:01:20Z jaguarondi $*/

/** \file commands.h
    \brief Firmware API
    \ingroup commands
*/

/** \defgroup commands Firmware interface (DEPRECATED)

    These firmware commands constitute the program interface between all CPUs
    and the computer. Both actions (also called commands) and status are
    transmitted by these commands.
 *  @{ */

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdint.h>
/*
 * Move commands
 */

#define BLINK_EYES_CMD      0x40        /* blink the eyes */
/* 1st parameter: number of movements before the eyes will stop */
#define STOP_EYES_CMD       0x32        /* stop the eyes motor */
#define OPEN_EYES_CMD       0x33        /* open the eyes if they are closed */
#define CLOSE_EYES_CMD      0x38        /* open the eyes if they are closed */

#define MOVE_MOUTH_CMD      0x41        /* move the mouth */
/* 1st parameter: number of movements before the mouth will stop */
#define OPEN_MOUTH_CMD      0x34        /* open the mouth if it is closed */
#define CLOSE_MOUTH_CMD     0x35        /* close the mouth if it is open */
#define STOP_MOUTH_CMD      0x36        /* stop the mouth motor */

#define WAVE_WINGS_CMD      0x80        /* move the wings up and down */
/* 1st parameter: number of movements before the wings will stop */
/* 2nd parameter: PWM value between 1 (slow) and 5 (fast) */
#define STOP_WINGS_CMD      0x30        /* stop the wings motor */
#define RESET_WINGS_CMD     0x31     /* reset the wings in the low position */
#define RAISE_WINGS_CMD     0x39     /* move the wings in the upper position */
#define LOWER_WINGS_CMD     0x3A     /* move the wings in the lower position */

#define SPIN_LEFT_CMD       0x82        /* spin left of a given angle */
/* 1st parameter: angle to turn, the unit is approximately 1/8th of a turn */
/* 2nd parameter: PWM value between 1 (slow) and 5 (fast) */
#define SPIN_RIGHT_CMD      0x83        /* spin right of a given angle */
/* 1st parameter: angle to turn, the unit is approximately 1/8th of a turn */
/* 2nd parameter: PWM value between 1 (slow) and 5 (fast) */
#define STOP_SPIN_CMD       0x37        /* stop the spinning motor */

/*
 * IR commands
 */

#define TURN_IR_ON_CMD      0x17        /* turn IR LED on */
#define TURN_IR_OFF_CMD     0x18        /* turn IR LED off */
#define IR_SEND_RC5_CMD     0x91        /* send RC5 ir code */
/* 1st parameter: address - xxTAAAAA (T: toggle bit, A: RC5 5 bits address) */
/* 2nd parameter: command - xxCCCCCC (C: RC5 6 bits command) */

/*
 * LED commands
 */

#define LED_ON_CMD          0x1A        /* Turn all LEDs on */
#define LED_OFF_CMD         0x1B        /* Turn all LEDs off */
#define LED_L_ON_CMD        0x1C        /* Turn left LED on */
#define LED_L_OFF_CMD       0x1D        /* Turn left LED off */
#define LED_R_ON_CMD        0x1E        /* Turn right LED on */
#define LED_R_OFF_CMD       0x1F        /* Turn right LED off */
#define LED_TOGGLE_CMD      0x9A        /* Toggle LEDs */
/* 1st parameter: number of LED toggles */
/* 2nd parameter: delay between each LED toggle, in multiple of 4ms */

/*
 * Audio commands
 *
 * Audio commands can't have 3 parameters, see the above note for details
 */
#define PLAY_SOUND_CMD 0x90        /* play a sound from the flash sound bank */
/* 1st parameter: sound number */
/* 2nd parameter: sound volume */
#define STORE_SOUND_CMD 0x52

#define CONFIRM_STORAGE_CMD 0x53
/* 1st parameter: 1 to write the sound
 *                0 to not write the sound*/

#define ERASE_FLASH_CMD 0x54

#define MUTE_CMD            0x92        /* mute/unmute the audio amplifier */
/* 1st parameter: mute state 0:unmute 1:mute */
/* 2nd parameter: reserved */

/**
 * Set tux in sleep mode.
 *
 * Parameters:
 *    - 1 : Sleep mode type
 *    - 2 : CMD (=0) or ACK (=1)
 */
#define SLEEP_CMD  0xB7
#define SLEEPTYPE_AWAKE 0 /* Go to sleep without running the actions. */
#define SLEEPTYPE_QUICK 1 /* Go to sleep without running the actions. */
#define SLEEPTYPE_NORMAL 2 /* Run the sleep actions before going to sleep. */
#define SLEEPTYPE_CONTINUE 3 /* Sleep actions done, continue to sleep. */
#define SLEEPTYPE_DEEP 4 /* Minimal consumption but can't wake up. */

/**
 * Wireless configuration of the frequency range to avoid.
 *
 * Parameters:
 *    - 1 : Lower frequency of the forbidden range
 *    - 2 : Higher frequency of the forbidden range
 */
#define WIRELESS_FREQ_BOUNDARIES_CMD 0x88

/*
 * Status commands
 */
#define STATUS_PORTS_CMD        0xC0    /* send core CPU's ports */
/* 1st parameter: PORTB
 *   PB0: wings motor backward
 *   PB1: spin motor forward
 *   PB2: spin moor backward
 *   PB3: mouth open position switch
 *   PB4: mouth closed position switch
 *   PB5: head push button
 *   PB6: charger inhibit signal
 *   PB7: external I/O
 */
/* 2nd parameter: PORTC
 *   PC0: phototransistor pull-up load
 *   PC1: wings position switch
 *   PC2: right blue LED
 *   PC3: left blue LED
 *   PC4: i2c SDA line
 *   PC5: i2c SCL line
 *   PC6: reset
 */
/* 3rd parameter: PORTD
 *   PD0: head motor for mouth
 *   PD1: head motor for eyes
 *   PD2: IR receiver signal
 *   PD3: spin position switch
 *   PD4: wings motor forward
 *   PD5: IR LED
 *   PD6: eyes open position switch
 *   PD7: eyes closed position switch
 */

/* send sensors status of the audio CPU to the core CPU */
#define SEND_AUDIOSENSORS_CMD 0xF0
/* send the playback status */
#define STATUS_SENSORS1_CMD 0xC1
/* 1st parameter: switches and status from the audio CPU
 *   .0: left wing push button
 *   .1: right wing push button
 *   .2: power plug insertion switch
 *   .3: head push button
 *   .4: charger LED status
 *   .5: RF connection status (0: disconnected, 1: connected)
 *   .6: internal power switch for the audio amplifier, motor drivers and IR
 *       LED
 *   .7: mute status (0: muted, 1: unmuted)
 */
#define STATUS_LEFTWINGBTN_MK       0x01
#define STATUS_RIGHTWINGBTN_MK      0x02
#define STATUS_POWERPLUGSW_MK       0x04
#define STATUS_HEADBTN_MK           0x08
#define STATUS_CHARGER_MK           0x10
#define STATUS_RF_MK                0x20
#define STATUS_VCC_MK               0x40
#define STATUS_MUTE_MK              0x80
/* 2nd parameter: sound played from the flash
 * 3rd parameter: audio activity
 *   .0: True if a sound is played from the flash memory, TTS or streaming.
 */

/* send the programming state from the audio CPU */
#define STATUS_FLASH_PROG_CMD       0xCD
/* 1st parameter: The current state (see flash.h)
 * 2nd parameter: The size of the last sound (1 = 4kB)
 */

#define STATUS_LIGHT_CMD            0xC2
/* 1st parameter: light level high byte */
/* 2nd parameter: light level low byte */
/* 3rd parameter: light mode: 0: low light, 1:strong light */

/* send position counters which is related to the motor status */
#define STATUS_POSITION1_CMD 0xC3
/* 1st parameter: eyes position counter */
/* 2nd parameter: mouth position counter */
/* 3rd parameter: wings position counter */

/* send position counters which is related to the motor status */
#define STATUS_POSITION2_CMD 0xC4
/* 1st parameter: spin position counter */
/* 2nd parameter: flippers position */
/* 3rd parameter: */

#define STATUS_IR_CMD               0xC5
/* 1st parameter: RC5 ir code received from tux's remote */
/*   .7: set when RC5 data has been received
 *   .6: toggle bit of the RC5 code
 *   .5-0: RC5 6 bits command) */
/* 2nd parameter: undefined */
/* 3rd parameter: undefined */

/**
 * ID command used to set tux's ID.
 * 
 * Send an ID value from 0x00 to 0xFE.
 *
 * Parameters:
 *    - 1 : Tux ID number
 *    - 2 : reserved
 */
#define SET_ID_CMD 0xB5

/**
 * Requests a connection to a tux given it's ID, or ID value sent back from the
 * currently connected tux.
 *
 * To wake up tux from the computer, the second parameter should be set to 1,
 * otherwise a tux which is asleep will only connect when it will wake up.
 * 
 * Parameters:
 *    - 1 : Tux ID number
 *    - 2 : Wake-up request
 */
#define CONNECT_ID_CMD 0xB6

#define STATUS_BATTERY_CMD            0xC7
/* 1st parameter: battery level high byte */
/* 2nd parameter: battery level low byte */
/* 3rd parameter: battery measure status : 0 - motors off; 1 - motors on*/

#define STATUS_AUDIO_CMD 0xCC
/* 1st parameter: The number of the played sound. If no sound played, 0 */
/* 2nd parameter: Programming steps :
 *                0 : no programming
 *                1 : Flash erased
 *                2 : TOC
 *                3 : Sounds track */
/* 3rd parameter: The track number which is programming*/

/*
 * Special commands
 */
/** Global error command */
#define GERROR_CMD       0xF9
/* 1st parameter: CPU number */
/* 2nd parameter: error type */
/* 3rd parameter: optional parameter */
enum gerrors
{
    GERROR_NONE = 0,
    GERROR_CMDINBUF_OVF,
    GERROR_CMDINBUF_EMPTY,
    GERROR_CMDOUTBUF_EMPTY,
    GERROR_CMDOUTBUF_FULL,
    GERROR_INV_RECEIVE_LENGTH,
    CMDGERROR_OUTBUF_OVF,
};

#define WAIT_CMD 0xFA
#define END_CMD 0xFB
#define FEEDBACK_CMD    0xF8
/* 3 paramters sent back depending on the last command sent to tux which is
 * supposed to send feedback */
#define PING_CMD        0x7F
/* 1st parameter: number of pongs to be replied */
#define PONG_CMD        0xFF
/* 1st parameter: number of pongs pending */
/* 2nd parameter: number of pongs lost by the I2C */
/* 3rd parameter: number of pongs lost by the RF link */
#define COND_RESET_CMD 0x3E /* reset conditional flags that are resettable */

/*
 * Error numbers
 */

enum error_numbers_t
{
    E_STACK_CORRUPTION,
    /* 3rd parameter: see source code, depends on the function that report the
     * error */
    E_STATUS_INVALID,
    /* 3rd parameter: invalid status value */
};

/*
 * Special parameters
 */

#define RESERVED        0

/*! @} */
#endif /* _COMMAND_H_ */
